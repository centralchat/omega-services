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
 *    $Id: g_modreload.c 2201 2012-01-08 07:08:09Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_modreload"
#define VERSION "$Id: g_modreload.c 2201 2012-01-08 07:08:09Z jer $"
#define AUTHOR "Twitch"



static void cmd_modreload (User*, int, char **);


static void help_gmodreload    (User*);
static void help_modreload_ext(User*);

static int  gbasic_load();
static void gbasic_close();


extern void do_restart(void);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;

      module_dependency("g_core");
	  ch = AddCmd(s_Guardian, "MODRELOAD",    ACC_FLAG_MODULE, cmd_modreload, help_modreload_ext, 1);
      AddHelp(s_Guardian,help_gmodreload);
      return MOD_CONT;


}


static void gbasic_close()
{

	DelCmd(s_Guardian,"MODRELOAD",   cmd_modreload, help_modreload_ext);
	DelHelp(s_Guardian,help_gmodreload);
	return;
}

static void cmd_modreload(User* u, int ac, char** params)
{
	int status;
	int i = 0;
	
	if  ((ac < 1) || (ac > 2)) 
	{
		sendto_one(s_Guardian,u,"Invalid syntax");
		sendto_one(s_Guardian,u," \002SYNTAX: MODRELOAD [MODULE]\002"); 
		return;
	}
	
	if (strcasecmp(params[0],"g_core")==0)
	{
		sendto_one(s_Guardian,u,"You can not perform any actions on this module");
		return;
	}
	sendto_logchan("\002Module:\002 Reloading module %s",params[0]);
	status = module_close(params[0]);
	if (status == 0)
	{
		status = module_open(params[0],MOD_TYPE_UNKNOWN);
		if (status == 0)
			sendto_one(s_Guardian,u,"Module reloaded successfully");
		else
			sendto_one(s_Guardian,u,"Unable to reload module - %s",GetModErr(status));
	}
	else
		sendto_one(s_Guardian,u,"Unable to reload module - %s",GetModErr(status));
	sendto_logchan("%s: %s",params[0], GetModErr(status));
	return;
}


/************************************************/
/**
 * Help functions
 */

static void help_gmodreload (User* u)
{
	sendto_one_help(s_Guardian,u, "MODRELOAD","Reload a module file.");
	return;
}

static void help_modreload_ext (User* u)
{
	sendto_one(s_Guardian,u,"\002MODRELOAD [MODULE]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u," Reload the module FILENAME to the server.");
	sendto_one(s_Guardian,u," ");
}



