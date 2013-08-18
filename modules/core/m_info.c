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
 *    $Id: m_info.c 1942 2010-06-25 07:25:07Z twitch $
 */

#include "stdinc.h"
#include "server.h"

#define NAME    "m_info"
#define AUTHOR  "Jer"
#define VERSION "$Id: m_info.c 1942 2010-06-25 07:25:07Z twitch $"

static void m_info (User *, int, char **);

extern IRCDProto IRCd;

static int m_info_load();
static void m_info_close();

/*****************************************/
/**
 *  Module_Init() - Module factory called
 *                when the module is loaded.
 *   @param Module* - pointer to the module struct
 *                  for this current module.
 *   return void
 */


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),m_info_load, m_info_close);

static int m_info_load()
{
    AddUserCmd ("INFO", m_info);
    return MOD_CONT;
}

static void m_info_close()
{
	DelUserCmd("INFO", m_info);
	return;
}

static void m_info  (User *u, int argc, char **argv)
{
    ircd_numeric ( u, 371, ":'-------------------oOo-------------------'");
    ircd_numeric ( u, 371, ":| %s(%s)-%s %s", PACKAGE_NAME, PACKAGE_VERSION, VERSION_STRING_DOTTED, CfgSettings.servername);
    ircd_numeric ( u, 371, ":|   ");
    ircd_numeric ( u, 371, ":|  * Protocol: %s (%s)", IRCd.name, IRCd.author);
    ircd_numeric ( u, 371, ":|  * Birth Date: %s, compile #%s",COMPILE_DATE,BUILD);
    ircd_numeric ( u, 371, ":|  * Compiled On: %s", SYSUNAME);
    ircd_numeric ( u, 371, ":|  * Configured with: %s", CONFIGURE_ARGS);
    ircd_numeric ( u, 371, ":|  * Module API: %d",API_VERSION);
    ircd_numeric ( u, 371, ":|  ");

#ifdef OMEGASTATS
	ircd_numeric ( u, 371, ":|  * Omega Statistical Reporting Enabled");
	ircd_numeric ( u, 371, ":|  ");
#endif

#ifdef USE_DEBUG_CMDS
    ircd_numeric ( u, 371, ":|  ");
    ircd_numeric ( u, 371, ":|      ***************************   ");
    ircd_numeric ( u, 371, ":|      *        DEBUG MODE       *   ");
    ircd_numeric ( u, 371, ":|      ***************************   ");
    ircd_numeric ( u, 371, ":|  ");
#endif

    ircd_numeric ( u, 371, ":|  ");
    ircd_numeric ( u, 371, ":'-----------------------------------------'");
    ircd_numeric ( u, 371, ":|   ");
    ircd_numeric ( u, 371, ":| \002Developers:\002");
    ircd_numeric ( u, 371, ":|  * twitch (mitch AT omega-services DOT org)");
    ircd_numeric ( u, 371, ":|  * jer (jeremy AT ssnet DOT ca)");
    ircd_numeric ( u, 371, ":|   ");
    ircd_numeric ( u, 371, ":| \002Contributors:\002");
    ircd_numeric ( u, 371, ":|  * Rob (no email)");
    ircd_numeric ( u, 371, ":|  * Stranded (radienlee@gmail.com) ");
    ircd_numeric ( u, 371, ":|   ");
    ircd_numeric ( u, 371, ":'-----------------------------------------'");
    ircd_numeric ( u, 374, ":End of /INFO");

    return;
}
