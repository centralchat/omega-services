/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
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
 *    $Id: protocol.c 2182 2012-01-05 02:13:53Z  $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME	"g_fantasy"
#define VERSION	"$Id: m_fantasy.c 1942 2010-06-25 07:25:07Z twitch $"
#define AUTHOR	"Jer"

static void fantasy_privmsg	(User *, int, char **);
static int  evh_rehash		(int, void*);


static int Module_Init();
static void Module_Close();

static char prefix;


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Init, Module_Close);


static int Module_Init ()
{
	char *ce;
	AddUserCmd ("PRIVMSG", fantasy_privmsg);
	if ((ce = (char *)get_config_entry("fantasy", "prefix")))	
		prefix = *ce;
	else
		prefix = '!';

	AddEvent("REHASH", evh_rehash);
	return MOD_CONT;
}

static void Module_Close ()
{
	DelUserCmd ("PRIVMSG", fantasy_privmsg);
	return;
}

static int evh_rehash(int ac, void *notused) { 
	char *ce;
	if ((ce = (char *)get_config_entry("fantasy", "prefix")))	
		prefix = *ce;
	else
		prefix = '!';
	return EVENT_CONT;
}


static void fantasy_privmsg (User *source, int argc, char **argv)
{
	int 		n = 0;
	char		*params[15];
	dlink_node  *dl;
	Channel 	*c;
	ChanUser 	*cu;
    char 		*ptr = NULL;
	char		*cmd = NULL;
	char 		*pos = NULL;
	
	if (argc < 3)
		return;
		
	if (*argv[1] != '#') 
		return;

    if (strlen(argv[2])<=1)
		return;
		
	if (*argv[2] != prefix)
		return;

	if (!(c = find_channel(argv[1])))
		return;

	cmd = strdup(argv[2]);
	if ((ptr = strchr((cmd++), ' ')) != NULL) {
		*ptr++ = '\0';
		while (1) {
			params[n++] = ptr;
		 	if ((pos = strchr(ptr, ' ')) != NULL) {
				params[n] = NULL;
				*pos++ = '\0';
				ptr = pos;
			} else 
				break;
		}
	}

	DLINK_FOREACH(dl, c->users.head) {
			cu = dl->data;
			if ((MyConnect(cu->user)) && (cu->user->fantasy)) 
				  do_cmd (cmd, cu->user, source, n, params);	
	}
	return;
}
