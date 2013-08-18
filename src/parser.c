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
 *    $Id: parser.c 2319 2012-01-31 06:47:03Z jer $
 */

#include "stdinc.h"
#include "server.h"
#include "parser.h"

static inline int string_to_array(char *string, const char **parv);
void parse(char *arv);

/**********************************************/
/**
  * Hook our socket so we can handle our own socket events
  */

int psr_init()
{
    AddUserCmd("PRIVMSG", HandleClientCmds);
	return 0;
}

/**********************************************/

static inline int
string_to_array(char *string, const char **parv)
{

    char *p;
	char *buf = string;
	int x = 1;


	//meh how did this happen?
	if ((!buf) || (!string))
		return 0;

	if(*buf == '\0')
		return x;

	do
	{
		if(*buf == ':')	/* Last parameter */
		{
			buf++;
			parv[x++] = buf;
			parv[x] = NULL;
			return x;
		}
		else
		{
			parv[x++] = buf;
			parv[x] = NULL;
			if((p = strchr(buf, ' ')) != NULL)
			{
				*p++ = '\0';
				buf = p;
			}
			else
				return x;
		}
		while (*buf == ' ')
			buf++;
		if(*buf == '\0')
			return x;
	}
	/* we can go up to parv[MAXPARA], as parv[0] is taken by source */
	while (x < MAXPARA);

	if(*p == ':')
		p++;

	parv[x++] = p;
	parv[x] = NULL;
	return x;
}

/**********************************************/


void parse(char *inbuf)
{

    Link* li;
    User* u;

	const char *para[MAXPARA + 2];

	char *command = NULL;
    char *s = NULL;
	char *pos = NULL;

    int i;
	int n = 0;

	memset(para,0,sizeof(para));

	if ((!inbuf) || (*inbuf == '\0'))
		return;

	alog(2,"RECV: %s", inbuf);

	for (n = 0; n < MAXPARA; n++)
		para[n] = NULL;

	if ((pos = strchr (inbuf, ' ')))
	{
		*pos = '\0';
		pos++;

		if (*inbuf == ':')
		{
			inbuf++;
			para[0]	= inbuf;

			if ((s = strchr(pos, ' ')))
			{
				*s = '\0';
				s++;

				command = pos;
			}
			else
			{
				s = NULL;
				command = pos;
			}
		}
		else
		{
			s = pos;
			command = inbuf;
		}
	} else {
		command = inbuf;
		s = NULL;
	}
	
	if (s)
		n = string_to_array (s, para);

	/* assert for safety, if command is NULL abort */
	s_assert(command != NULL);

    for (i = 1; para[i] != NULL; i++)
        alog (LOG_DEBUG3, "para[%d]: %s", i, para[i]);

    if (IRCd.p10) {
        /* P10 does not prefix sources with : */
        li  = find_serv (command);
        u   = find_uid  (command);

        if (li || u)
            para[0] = command;

        if (!li && !u)
            alog (LOG_DEBUG2, "No source found, assuming uplink for command %s", command);

        if (li)
            HandleServCmd ((char *)para[1], li, n, (char **) para);
        else if (u)
            HandleUserCmd ((char *)para[1], u, n, (char **) para);
        else
            HandleServCmd (command, NULL, n, (char **) para);

        return;
    }

       //okay here's a few things we need to keep in mind
       //PARA[0] is not always a server s<>s allows for it
       //to be null if CMD is directly from the uplink server

       //always assume this is from the uplink
	if (!para[0])
		HandleServCmd (command, NULL, n, (char **)para);

	else
	{
		if ((li = find_serv((char*)para[0])))
		{
			HandleServCmd (command, li, n, (char **)para); //handles a command from the server
			return;
		}

		if ((u = find_user((char*)para[0])))
        {
			u->lastsent = time(NULL); //set our time so we can have accurate WHOIS
			HandleUserCmd(command, u, n, (char**)para);

			return;
		}

		if (n > 2)
			if (((strcasecmp(command, "NOTICE")) == 0) && ((strcasecmp (para[1], "AUTH")) == 0))
				return;

		alog (LOG_DEBUG, "Unknown prefix: %s\n", para[0]);
//		fprintf (stderr, "Unknown prefix: %s\n", para[0]);
		return;

	}

	command = NULL;
    s = NULL;
	pos = NULL;

	memset(para,0,sizeof(para));

	return;
}

/**********************************************/
/**
  * break_params()
  * Breaks argv up starting starting at start and ending at break
  * break can be argc it will halt auto if it is :)
  */


static inline int break_params(int argc, char **argv,const char **para,int start,int brk)
{
	int i = 1;
       int cnt = 0;

	memset(para,0,sizeof(para));
	while (i <= argc)
	{
		if (i >= start)
		{
			para[cnt] = (char*) argv[i];
			cnt++;
		}
		if (i >= brk)
			break;
		i++;
	}
	return 0;
}


/**********************************************/

void psr_HandleEvent(Socket* s,int et)
{
    char	*line;

	switch (et)
	{
		case EVENT_READ:
				while (1)
				{
					line = GetLine (s);

					if (line == NULL)
						break;
						
					parse (line);
					free(line);
				}
		break;
	}

}




