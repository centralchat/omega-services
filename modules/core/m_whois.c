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

#define NAME "m_whois"
#define AUTHOR "Jer"
#define VERSION "$Id: m_whois.c 1942 2010-06-25 07:25:07Z twitch $"

enum MYVERSION {
       MAJOR = 0,
       MINOR = 4,
       PATCH = 0,

};


int  Module_Init();
void Module_Close();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Init, Module_Close);

void m_whois (User *user,int i,char **argv);

int Module_Init()
{

	AddUserCmd("WHOIS", m_whois);
	return MOD_CONT;
}

void Module_Close ()
{
	DelUserCmd("WHOIS", m_whois);
	return;
}


void m_whois (User *source, int argc, char **argv)
{
	dlink_node		*dl;
	User			*user;

	if (!(user = find_user (argv[2])))
	{
		return;
	}

	send_line (":%s 311 %s %s %s %s * :%s", CfgSettings.servername, source->nick, user->nick, user->user,
				user->host, user->realname);
	send_line (":%s 312 %s %s %s :%s", CfgSettings.servername, source->nick, user->nick, user->serv->name, CfgSettings.desc);
	send_line (":%s 317 %s %s %lu %lu :seconds idle, signon time", CfgSettings.servername, source->nick,
				user->nick, (unsigned long)(time(NULL) - user->lastsent), (unsigned long)starttime);
	send_line (":%s 318 %s %s :End of /WHOIS list.", CfgSettings.servername, source->nick, user->nick);

    return;
}

