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
 *    $Id: g_modlist.c 2341 2012-02-07 07:19:48Z twitch $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_modlist"
#define VERSION "$Id: g_modlist.c 2341 2012-02-07 07:19:48Z twitch $"
#define AUTHOR "Twitch"



static void cmd_modlist (User*, int, char **);


static void help_gmodlist    (User*);
static void help_modlist_ext(User*);

static int  gbasic_load();
static void gbasic_close();


extern void do_restart(void);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;

      module_dependency("g_core");
      AddCmd(s_Guardian, "MODLIST",    ACC_FLAG_MODULE, cmd_modlist, help_modlist_ext, 1);
      AddHelp(s_Guardian,help_gmodlist);
      return MOD_CONT;


}


static void gbasic_close()
{

	DelCmd(s_Guardian,"MODLIST",   cmd_modlist, help_modlist_ext);
	DelHelp(s_Guardian,help_gmodlist);
	return;
}

static void cmd_modlist(User* u, int ac, char** params) {
        Module * m;
        dlink_node * dl;
        int i = 0, s_mod = 0;
        char padding[20];
        char *ptr = NULL;

        sendto_one(s_Guardian,u,"Current module list:");
        i = 0;

       /*
	* Loop through out module list and list in order loaded...
        * our module loader is WAY to basic to make me happy, 
        * XXX- we need to be able to set MOD_VENDOR or stuff 
        * so we can better associate our modules
   	*/
   
        DLINK_FOREACH(dl,modules.head)
        {
                m = dl->data;
                i++;
                ptr = padding;
		for (s_mod = (strlen(m->name)+3); s_mod < sizeof(padding); s_mod++) { 
			*ptr = ' ';
			ptr++;
		}
                sendto_one(s_Guardian,u, "\002%s\002%s [%lu] [%p]",m->name, padding, m->mi->api, m);
		memset(padding, 0, sizeof(padding));
        }

        sendto_one(s_Guardian,u,"There are currently \002%d\002 modules loaded.",i);
        return;

}


/************************************************/
/**
 * Help functions
 */

static void help_gmodlist (User* u)
{
	sendto_one_help(s_Guardian,u, "MODLIST", "List all currently loaded modules..");
	return;
}

static void help_modlist_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002MODLIST\002");
    sendto_one(s_Guardian,u," ");
    sendto_one(s_Guardian,u,"     List all currently loaded modules, and their vendors.");
    sendto_one(s_Guardian,u," ");
    return;

}



