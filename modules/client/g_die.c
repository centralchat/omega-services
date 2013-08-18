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
 *    $Id: g_die.c 2306 2012-01-30 00:42:25Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_core"
#define VERSION "$Id: g_die.c 2306 2012-01-30 00:42:25Z jer $"
#define AUTHOR "Twitch"


static void cmd_shutdown  (User*, int, char **);
static void cmd_modlist   (User*, int, char **);
static void cmd_modload   (User*, int, char **);
static void cmd_modunload (User*, int, char **);
static void cmd_modreload (User*, int, char **);

static void cmd_access    (User*, int, char **);
static void cmd_rehash    (User*, int, char **);
static void cmd_restart   (User*, int, char **);
static void cmd_raw	  (User*, int, char **);

static void cmd_update    (User*, int, char **);

static void help_gcore     (User*);


static void help_shutdown_ext (User*);
static void help_restart_ext  (User*);
static void help_rehash_ext   (User*);
static void help_access_ext   (User*);
static void help_modlist_ext  (User*);
static void help_modload_ext  (User*);
static void help_modreload_ext(User*);
static void help_modunload_ext(User*);

static void help_raw_ext      (User*);

static int  gbasic_load();
static void gbasic_close();


extern void do_restart(void);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;

      module_dependency("g_core");

      ch = AddCmd(s_Guardian, "SHUTDOWN",  ACC_FLAG_DIE, cmd_shutdown, help_shutdown_ext, 1);
      ch = AddCmd(s_Guardian, "REHASH",    ACC_FLAG_REHASH, cmd_rehash, help_rehash_ext, 1);
      ch = AddCmd(s_Guardian, "RESTART",   ACC_FLAG_DIE, cmd_restart, help_restart_ext, 1);
#ifdef DEBUG_MODE  
       ch = AddCmd(s_Guardian, "RAW", ACC_FLAG_SUPER, cmd_raw, help_raw_ext, 1);
#endif
      AddHelp(s_Guardian,help_gcore);
      return MOD_CONT;


}


static void gbasic_close()
{
	DelCmd(s_Guardian,"SHUTDOWN",  cmd_shutdown,help_shutdown_ext);
	DelCmd(s_Guardian,"REHASH",    cmd_rehash,  help_rehash_ext);
	DelCmd(s_Guardian,"RESTART",   cmd_restart, help_restart_ext);
#ifdef DEBUG_MODE  
        DelCmd(s_Guardian, "RAW",  cmd_raw, help_raw_ext);
#endif	

	DelHelp(s_Guardian,help_gcore);

	return;

}

/***************************************************/
static void cmd_raw(User* u, int ac, char** av)
{
	char param[512];
	ExpandParv(param,512,0,ac,av);
	send_line(param);
	ircd_wallop(s_Guardian->nick, "\002%s\002 (\002%s@%s\002) has used raw command \002%s\002.", 
				u->nick, u->user, u->host, param);
}

/***************************************************/

static void cmd_rehash(User* u, int ac, char** av)
{
	ircd_wallop(s_Guardian->nick,"%s is rehashing the server",u->nick);
	Rehash(0);
	return;
}

/****************************************************/


//you know its safe to say we can destroy this buffer.

static void cmd_modlist(User* u, int ac, char** params) {
	Module * m;
	dlink_node * dl;
	int i = 0 ;
	
	sendto_one(s_Guardian,u,"Current module list:");
	i = 0;

	DLINK_FOREACH(dl,modules.head)
	{
		m = dl->data;

		i++;

		sendto_one(s_Guardian,u, "%s [%s]",m->name,m->mi->version);

	}

	sendto_one(s_Guardian,u,"There is currently \002%d\002 modules loaded.",i);
	return;
	
}
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
	
	if (strcasecmp(params[0],"m_guardian")==0)
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
static void cmd_modload(User* u, int ac, char** params)
{
	int status;
	int i = 0;
	
	if  ((ac < 1) || (ac > 2)) 
	{
		sendto_one(s_Guardian,u,"Invalid syntax");
		sendto_one(s_Guardian,u," \002SYNTAX: MODLOAD [MODULE]\002"); 
		return;
	}
	
            sendto_logchan("\002Module:\002 Loading module %s",params[0]);

            status = module_open(params[0],MOD_TYPE_UNKNOWN);

            sendto_one(s_Guardian,u,"Loading complete - %s",GetModErr(status));

            if (status == MOD_CONT)
                	ircd_wallop(s_Guardian->nick,"%s has loaded module %s",u->nick,params[0]);

             sendto_logchan("%s: %s",params[0],GetModErr(status));
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
	
	if (strcasecmp(params[0],"m_guardian")==0)
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



/***************************************************/


static void cmd_shutdown(User *u, int ac, char **av)
{
	ircd_wallop(s_Guardian->nick,"Shutdown CMD issued by %s (%s@%s)",u->nick,u->user,u->host);
	alog(0,"Shutdown CMD issued from the server by %s (%s@%s)",u->nick,u->user,u->host);
	Exit(0);
	return;

}



/**********************************************/

static void cmd_access(User* u, int ac, char** params)
{
	dlink_node *dl,*hdl;
	Access* ca;
	char* host;

	DLINK_FOREACH(dl,accesslist.head)
	{
	    ca = dl->data;

		sendto_one(s_Guardian,u,"%s [%s]",ca->name,ca->flags);
		DLINK_FOREACH(hdl,ca->hosts.head)
		{
				host = hdl->data;
				sendto_one(s_Guardian,u,"- %s",host);

		}
	}
	return;
}





/************************************************/


static void cmd_restart(User* u, int ac, char** av)
{
	do_restart();
	return;
}

/************************************************/
/**
 * Help functions
 */

static void help_gcore (User* u)
{
		
		sendto_one_help(s_Guardian,u, "SHUTDOWN", "Clean up then shutdown the server.");
		sendto_one_help(s_Guardian,u, "RESTART",     "Restart the server");
		sendto_one_help(s_Guardian,u, "REHASH",      "Reload all configuration files");
#ifdef DEBUG_MODE
		sendto_one_help(s_Guardian,u, "RAW",   "Send a raw command to the server.");
#endif	
	return;
}

static void help_raw_ext(User* u) 
{
	sendto_one(s_Guardian,u,"Syntax: \002RAW [STRING]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     Send raw string to a server.");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u," \002NOTE:\002");
	sendto_one(s_Guardian,u,"  THIS COMMAND IS POTENTIALLY ");
        sendto_one(s_Guardian,u,"  DEVASTATING TO A NETWORK,   ");
	sendto_one(s_Guardian,u,"  AND IS NOT INTENDED FOR USE ");
	sendto_one(s_Guardian,u,"  ON A PRODUCTION NETWORK.    ");
	sendto_one(s_Guardian,u," ");
	return;
}

static void help_shutdown_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002SHUTDOWN\002");
	sendto_one(s_Guardian,u,"");
	sendto_one(s_Guardian,u,"     Causes server to cleanup and shutdown.");
	sendto_one(s_Guardian,u,"");
	return;
}

static void help_restart_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002RESTART\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     Causes server to cleanup and restart its services.");
	sendto_one(s_Guardian,u," ");
	return;
}

static void help_rehash_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002REHASH\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     Reload all configuration files");
	sendto_one(s_Guardian,u," ");
	return;
}

static void help_access_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002ACCESS [OPTION]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"\002ACCESS LIST\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     List the entries in the current internal access list");
	sendto_one(s_Guardian,u," ");
	return;
}

/**
 * MODULE COMMANDS
 */
 
static void help_modlist_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002MODLIST\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     List all currently loaded modules, and their vendors.");
	sendto_one(s_Guardian,u," ");
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

static void help_modreload_ext (User* u)
{
	sendto_one(s_Guardian,u,"\002MODRELOAD [MODULE]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u," Reload the module FILENAME to the server.");
	sendto_one(s_Guardian,u," ");
}



