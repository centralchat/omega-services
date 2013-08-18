/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *    $Id: tools.c 2292 2012-01-29 09:18:09Z twitch $
 */

#include "stdinc.h"

#include "server.h"

#define TIMEBUF         100 /* Accepts time formatting */

/* These items are for the timediff() function, used to process time
** quantities (3 days, 45 minutes, 21 seconds)
*/
#define T_DAY       "day"
#define T_DAYS      "days"
#define T_HOUR      "hour"
#define T_HOURS     "hours"
#define T_MINUTE    "minute"
#define T_MINUTES   "minutes"
#define T_SECOND    "second"
#define T_SECONDS   "seconds"

static char current_uid[10];

static char p10_uid[7];
char        p10_id[3];

//p10_uid[0]  = '\0';

char * strtolower (char *str)
{
    int cnt;
    static char str_lower[1024];

    memset(str_lower,0,sizeof(str_lower));

    for (cnt = 0; *str && cnt < sizeof(str_lower); cnt++, str++);
          str_lower[cnt] = tolower(*str);


    return (char *) str_lower;
}


/**
 * Since not every one has strlcpy and strlcat we need to create them if
 * they do not exist on the system
 */

#ifndef HAVE_STRLCAT
size_t strlcat(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz, dlen;

  while (n-- != 0 && *d != '\0')
    d++;
  dlen = d - dst;
  n = siz - dlen;

  if (n == 0)
    return (dlen + strlen(s));
  while (*s != '\0')
  {
    if (n != 1)
    {
      *d++ = *s;
      n--;
    }
    s++;
  }
  *d = '\0';
  return (dlen + (s - src));       /* count does not include NULL */
}
#endif

#ifndef HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz)
{
  char *d = dst;
  const char *s = src;
  size_t n = siz;
  /* Copy as many bytes as will fit */
  if (n != 0 && --n != 0)
  {
    do
    {
      if ((*d++ = *s++) == 0)
        break;
    }
    while (--n != 0);
  }
  /* Not enough room in dst, add NULL and traverse rest of src */
  if (n == 0)
  {
    if (siz != 0)
      *d = '\0';              /* NULL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1);    /* count does not include NULL */
}
#endif

void init_uid (void)
{
	int	i;

	for (i = 0; i < 3; i++)
		current_uid[i]	= CfgSettings.sid[i];

	for (i = 3; i < 9; i++)
		current_uid[i] = 'A';

	current_uid[9] = '\0';

    p10_uid[0]  = '\0';
}

char * generate_uid (void)
{
	int	i;

	for (i = 8; i > 3; i--)
	{
		if (current_uid[i] == 'Z')
		{
			current_uid[i] = '0';
			return current_uid;
		}
		else if (current_uid[i] != '9')
		{
			current_uid[i]++;
			return current_uid;
		}
		else
			current_uid[i] = 'A';
	}

	if (current_uid[3] == 'Z')
	{
		current_uid[i] = 'A';
		s_assert (0);
	}
	else
		current_uid[i]++;

	return current_uid;
}

void generate_p10_uid (User *u) {
    int     i;

    if (p10_uid[0] == '\0')    // Init the uid list
    {
        alog (LOG_DEBUG2, "Initializing p10_uid");
        for (i = 0; i < 2; i++)
            p10_uid[i]  = p10_id[i];
        for (i = 2; i < 5; i++)
            p10_uid[i]  = 'A';

        p10_uid[6] = '\0';

        strlcpy (u->uid, p10_uid, 6);
        return;
    }

    alog (LOG_DEBUG2, "p10_uid: %s", p10_uid);

    for (i = 4; i > 2; i--)
    {
        if (p10_uid[i] == 'Z')
        {
            p10_uid[i] = '0';
            strlcpy (u->uid, p10_uid, 6);
            return;
        }
        else if (p10_uid[i] != '9')
        {
            p10_uid[i]++;
            strlcpy (u->uid, p10_uid, 6);
            return;
        }
        else
            p10_uid[i] = 'A';
    }

    if (p10_uid[2] == 'Z')
    {
        p10_uid[i] = 'A';
        s_assert (0);
    }
    else
        p10_uid[i]++;

    strlcpy (u->uid, p10_uid, 6);
    return;
}

void ExpandParv(char *dest, int max, int initial, int parc, char *parv[])
{
	 int i;

	if (parv[initial])
		strlcpy(dest, parv[initial], max);

	for (i = initial + 1; i < parc; i++)
	{
		strlcat(dest, " ", max);
		strlcat(dest, parv[i], max);
	}
}

int generictoken(char delim, int size, char *message, char **parv)
{
  char *next;
  int count;

  if (!message)
  {
    /* Something is seriously wrong...bail */
    parv[0] = NULL;
    return 0;
  }

  /* Now we take the beginning of message and find all the spaces...
  ** set them to \0 and use 'next' to go through the string
  */
  next = message;
  parv[0] = next;
  count = 1;

  while (*next)
  {
    /* This is fine here, since we don't have a :delimited
    ** parameter like tokenize
    */
    if (count == size)
      return count;
   
 
    if (*next == delim)
    {
      *next = '\0';
      next++;
      /* Eat any additional delimiters */
      while (*next == delim)
        next++;
      while (*next == ' ')
	next++; //eat whitespaces  
      /* If it's the end of the string, it's simply
      ** an extra space at the end.  Here we break.
      */
      if (*next == '\0')
        break;
      parv[count] = next;
      count++;
    }
    else
    {
      next++;
    }
  }

  return count;
}


uint32 hash(char *item)
{

  uint32  hashstr, i;
  hashstr = 0; i = 0;
  for (hashstr=0, i=0; i < strlen(item); i++)
  {
    hashstr += tolower(item[i]);
    hashstr += (hashstr << 10);
    hashstr ^= (hashstr >> 5);
  }

  hashstr += (hashstr << 3);
  hashstr ^= (hashstr >> 11);
  hashstr += (hashstr << 15);

  return hashstr;

}


//aim to protect against poor hash table logic.
//borrowed from java hash table source - Twitch
unsigned int hash_safe(char *item, ub4 len)
{
    unsigned int i = (unsigned int) hash(item);
    i += ~(i << 9);
    i ^=  ((i >> 14) | (i << 18)); /* >>> */
    i +=  (i << 4);
    i ^=  ((i >> 10) | (i << 22)); /* >>> */
    return (unsigned int) (i % len);

}

//convert human readable time values to seconds such as
//1d2h44m = 1 day 2 hours 44 minutes (were month = n)
time_t time_to_sec(char *time) {
	char *start;
	int cur_time = 0;
	int time_tmp = 0;
	char tmp[10];
	
	for (;*time; time++)
	{
		start = time;
		memset(tmp, 0, sizeof(tmp));
		
		while ((*time >= '0' && *time <= '9'))
			time++;
		
		switch (*time)
		{
			case 's':
					strncpy(tmp,start, time - start);
					cur_time += atoi(tmp);
			case 'm':
					strncpy(tmp,start, time - start);
					cur_time += (atoi(tmp) * 60);
					break;
			case 'h':
					strncpy(tmp,start, time - start);
					cur_time += (atoi(tmp) * 3600);
					break;
			case 'd':
					strncpy(tmp,start, time - start);
					cur_time += (atoi(tmp) * 86400);
					break;
			case 'n':
					strncpy(tmp,start, time - start);
					cur_time += (atoi(tmp) * 2629743);
					break;
			case 'y':
					strncpy(tmp,start, time - start);
					cur_time += (atoi(tmp) * 31556926);
					break;
			default:
					break;
		}
		
	}
	return (time_t) cur_time;
}


/* print a period of seconds into human readable format */
char *timediff(time_t u)
{
  static char buf[TIMEBUF];

  if (u > 86400)
  {
    time_t days = u / 86400;

    snprintf(buf, TIMEBUF, "%ld %s, %02ld:%02ld",
             (long)days, (days == 1) ? T_DAY : T_DAYS,
             (long)((u / 3600) % 24), (long)((u / 60) % 60));
  }
  else if (u > 3600)
  {
    time_t hours = u / 3600;
    time_t minutes = (u / 60) % 60;

    snprintf(buf, TIMEBUF, "%ld %s, %ld %s",
             (long)hours, (hours == 1) ? T_HOUR : T_HOURS, (long)minutes,
             (minutes == 1) ? T_MINUTE : T_MINUTES);
  }
  else
  {
    time_t minutes = u / 60;
    time_t seconds = u % 60;

    snprintf(buf, TIMEBUF,
             "%ld %s, %ld %s",
             (long)minutes, (minutes == 1) ? T_MINUTE : T_MINUTES,
             (long)seconds, (seconds == 1) ? T_SECOND : T_SECONDS);
  }

  return buf;
}


/**
 * Change an unsigned string to a signed string, overwriting the original
 * string.
 * @param str input string
 * @return output string, same as input string.
 */

char *str_signed(unsigned char *str)
{
    char *nstr;

    nstr = (char *) str;
    while (*str) {
        *nstr = (char) *str;
        str++;
        nstr++;
    }

    return nstr;
}



/* Equivalent to inet_ntoa */

void ntoa(struct in_addr addr, char *ipaddr, int len)
{

    unsigned char *bytes = (unsigned char *) &addr.s_addr;

    snprintf(ipaddr, len, "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2],
             bytes[3]);

    return;

}

/**
 * return an allocated string similar to inet_ntoa however
 * this doesn't take a length or char* argument however
 * this will only malloc up to 17 which is the length of an
 * IP addr + 1 padding. so we don't piss away memory :)
 */



char *str_ntoa(struct in_addr addr)
{

    char ipaddr[17];

    unsigned char *bytes = (unsigned char *) &addr.s_addr;

    snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", bytes[0], bytes[1], bytes[2],
             bytes[3]);

    return (char*) strdup(ipaddr);
}

void reverse_ip(const char *ip, char *result)
{

        struct sockaddr_in sa;
        inet_pton(AF_INET, ip, &(sa.sin_addr));
        unsigned char *bytes = (unsigned char *) &(sa.sin_addr);
        sprintf(result, "%u.%u.%u.%u", bytes[3], bytes[2], bytes[1], bytes[0]);
}
 


