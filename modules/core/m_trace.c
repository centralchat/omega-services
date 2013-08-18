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

#define NAME "m_trace"
#define AUTHOR "Jer"
#define VERSION "$Id: m_trace.c 1942 2010-06-25 07:25:07Z twitch $"


int Module_Init();
void Module_Close();

void m_trace (User *user,int i,char **argv);

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Init, Module_Close);


int Module_Init()
{
    AddUserCmd("TRACE", m_trace);
    return MOD_CONT;
}

void Module_Close()
{

	DelUserCmd("TRACE", m_trace);
	return;
}

void m_trace (User *user,int i,char **argv)
{
	dlink_node		*dl;
	User			*u;


	if (user->oper)
	{
		DLINK_FOREACH (dl, userlist.head)
		{
			u	= dl->data;

			if (MyConnect(u))
				send_line (":%s %d %s User users %s[%s@%s] (%s) 0 :0", CfgSettings.servername, 205, user->nick,
						u->nick, u->user, u->host, "*");
		}
	}

    return;
}

