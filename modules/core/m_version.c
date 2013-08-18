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

#define NAME "m_version"
#define AUTHOR "Twitch"
#define VERSION "$Id: m_version.c 1942 2010-06-25 07:25:07Z twitch $"


int Module_Init();
void Module_Close();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, Module_Close);

extern IRCDProto IRCd;

void m_version (User *user,int i,char **argv);

int Module_Init()
{

    AddUserCmd("VERSION", m_version);

	return MOD_CONT;
}

void Module_Close()
{
	DelUserCmd("VERSION", m_version);
	return;
}
void m_version (User *user,int i,char **argv)
{
    sendto_one_numeric (CfgSettings.servername, user, 351, ":%s(%s)-%s#%s %s :%s", PACKAGE_NAME, PACKAGE_VERSION, VERSION_STRING_DOTTED,
                        BUILD, CfgSettings.servername, (IRCd.ts > 0)? "TS" : "");

    return;
}


