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

	const char *para[MAXPARA + 2];

	char *command = NULL;
    char *s = NULL;
	char *pos = NULL;

    int i;
	int n = 0;

	memset(para,0,sizeof(para));

	if ((!inbuf) || (*inbuf == '\0'))
		return;

	log_message(LOG_DEBUG2,"RECV: %s", inbuf);

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
		n = parser_string_to_array (s, para);

	/* assert for safety, if command is NULL abort */
	s_assert(command != NULL);

    for (i = 1; para[i] != NULL; i++)
        log_message (LOG_DEBUG3, "para[%d]: %s", i, para[i]);

	command = NULL;
    s = NULL;
	pos = NULL;

	memset(para,0,sizeof(para));

	return;
}


