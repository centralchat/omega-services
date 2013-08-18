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
 *    $Id: g_update.c 2306 2012-01-30 00:42:25Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_update"
#define VERSION "$Id: g_update.c 2306 2012-01-30 00:42:25Z jer $"
#define AUTHOR "Twitch"



static void cmd_update (User*, int, char **);


static void help_gupdate    (User*);
static void help_update_ext(User*);

static int  gbasic_load();
static void gbasic_close();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;

      module_dependency("g_core");
      ch = AddCmd(s_Guardian, "UPDATE",    ACC_FLAG_OPER, cmd_update, help_update_ext, 1);
      ch = AddCmd(s_Guardian, "SAVE",	   ACC_FLAG_OPER, cmd_update, help_update_ext, 1);
      AddHelp(s_Guardian,help_gupdate);
      return MOD_CONT;


}


static void gbasic_close()
{

	DelCmd(s_Guardian,"UPDATE",   cmd_update, help_update_ext);
	DelHelp(s_Guardian,help_gupdate);
	return;
}

static void cmd_update(User* u, int ac, char** params) {
		Event("UPDATEDB", 0, NULL);
		Event("SAVEDB", 0, NULL);
		sendto_one(s_Guardian, u, "Updating databases.");
		ircd_wallop(s_Guardian->nick,"%s has forced update of services database.",u->nick);
        return;

}


/************************************************/
/**
 * Help functions
 */

static void help_gupdate (User* u)
{
	sendto_one_help(s_Guardian,u, "UPDATE","Force update of all services databases.");
	return;
}

static void help_update_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002UPDATE\002");
    sendto_one(s_Guardian,u," ");
    sendto_one(s_Guardian,u,"     Force services to write out database files, and update internal DB lists.");
    sendto_one(s_Guardian,u," ");
    return;

}



