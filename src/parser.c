/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2013 Omega Dev Team
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

#include "stdinclude.h"
#include "appinclude.h"
 
/**********************************************/

static inline int
parser_string_to_array(char *string, const char **parv)
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


void parser_handle_line(char *inbuf)
{

    // Link* li;
    // User* u;

	const char *parv[MAXPARA + 2];

	char *command = NULL;
  char *s       = NULL;
	char *pos     = NULL;

	args_t * args;  


  int i    = 0;
  int cnt  = 0;
	int parc = 0;


	void * source; 
	int    source_type = 0;

	memset(parv,0,sizeof(parv));

	if ((!inbuf) || (*inbuf == '\0'))
		return;

	log_message(LOG_DEBUG2,"RECV: %s", inbuf);

	for (cnt = 0; cnt < MAXPARA; cnt++)
		parv[cnt] = NULL;

	if ((pos = strchr (inbuf, ' ')))
	{
		*pos = '\0';
		pos++;

		if (*inbuf == ':')
		{
			inbuf++;
			parv[0]	= inbuf;

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
		parc = parser_string_to_array (s, parv);

	/* assert for safety, if command is NULL abort */
	s_assert(command != NULL);


	//Dont go any further ignore NOTICE AUTH
	if (parc > 2)
	{
		if (((strcasecmp(command, "NOTICE")) == 0) && ((strcasecmp (parv[1], "AUTH")) == 0))
			return;
	}

  for (i = 1; core.debug && parv[i] != NULL; i++)
    log_message (LOG_DEBUG3, "para[%d]: %s", i, parv[i]);


	if ((!parv[0]) || ((source = server_findby_name(parv[0]))))
		source_type = 0;
	else
		source_type = 1;


  ARGS(args, source, parc, parv);
  if (source_type)
  	/* Do nothing for now */ ;
	else
		command_server_emit (command, args);

	// else
	// {
	// 	if ((li = find_serv((char*)para[0])))
	// 	{
	// 		HandleServCmd (command, li, n, (char **)para); //handles a command from the server
	// 		return;
	// 	}
	// 	if ((u = find_user((char*)para[0])))
    //  {
	// 		u->lastsent = time(NULL); //set our time so we can have accurate WHOIS
	// 		HandleUserCmd(command, u, n, (char**)para);
	// 		return;
	// 	}
	// 	if (n > 2)
	// 		if (((strcasecmp(command, "NOTICE")) == 0) && ((strcasecmp (para[1], "AUTH")) == 0))
	// 			return;
	// 	alog (LOG_DEBUG, "Unknown prefix: %s\n", para[0]);
	// 	return;
	// }
	command = NULL;
        s = NULL;
	    pos = NULL;

	memset(parv,0,sizeof(parv));

	return;
}


