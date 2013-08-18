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
 *    $Id: g_modunload.c 2201 2012-01-08 07:08:09Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_modunload"
#define VERSION "$Id: g_modunload.c 2201 2012-01-08 07:08:09Z jer $"
#define AUTHOR "Twitch"


static void cmd_modunload (User*, int, char **);
static void help_gmodunload (User*);
static void help_modunload_ext(User*);

static int  gbasic_load();
static void gbasic_close();


extern void do_restart(void);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;
      module_dependency("g_core");
      ch = AddCmd(s_Guardian, "MODUNLOAD", ACC_FLAG_MODULE, cmd_modunload, help_modunload_ext, 1);
      AddHelp(s_Guardian,help_gmodunload);
      return MOD_CONT;
}


static void gbasic_close()
{

	DelCmd(s_Guardian,"MODUNLOAD", cmd_modunload, help_modunload_ext);
	DelHelp(s_Guardian,help_gmodunload);

	return;

}

/**********************/

static void cmd_modunload(User* u, int ac, char** params)
{
	int status;
	int i = 0;
	
	if  ((ac < 1) || (ac > 2)) 
	{
		sendto_one(s_Guardian,u,"Invalid syntax");
		sendto_one(s_Guardian,u," \002SYNTAX: MODUNLOAD [MODULE]\002"); 
		return;
	}
	
	if (strcasecmp(params[0],"g_core")==0)
	{
		sendto_one(s_Guardian,u,"You can not perform any actions on this module");
		return;
	}
	sendto_logchan("\002Module:\002 Unloading module %s",params[0]);
	status = module_close(params[0]);
	sendto_one(s_Guardian,u,"Unloading complete - %s",GetModErr(status));
	sendto_logchan("%s: %s",params[0],GetModErr(status));
	return;
}
/**
 * Help functions
 */

static void help_gmodunload (User* u)
{	
	sendto_one_help(s_Guardian,u, "MODUNLOAD","Unload a module file.");	
	return;
}

static void help_modunload_ext (User* u)
{
	sendto_one(s_Guardian,u,"\002MODUNLOAD\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u," 	Unload the module named FILENAME from the server");
	sendto_one(s_Guardian,u," ");
}

static void help_modload_ext (User* u)
{
	sendto_one(s_Guardian,u,"\002MODLOAD [MODULE]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u," Load the module FILENAME.");
	sendto_one(s_Guardian,u," ");
}



