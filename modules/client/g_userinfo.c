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
 *    $Id: g_userinfo.c 2201 2012-01-08 07:08:09Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "g_userinfo"
#define VERSION "$Id: g_userinfo.c 2201 2012-01-08 07:08:09Z jer $"
#define AUTHOR "Twitch"



static void cmd_guserinfo (User*, int, char **);


static void help_guserinfo    (User*);
static void help_guserinfo_ext(User*);

static int  gbasic_load();
static void gbasic_close();


extern void do_restart(void);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;

      module_dependency("g_core");
	  ch = AddCmd(s_Guardian, "USERINFO",    ACC_FLAG_LIST, cmd_guserinfo, help_guserinfo_ext, 1);
      AddHelp(s_Guardian,help_guserinfo);
      return MOD_CONT;


}


static void gbasic_close()
{

	DelCmd(s_Guardian,"USERINFO",   cmd_guserinfo, help_guserinfo_ext);
	DelHelp(s_Guardian,help_guserinfo);
	return;
}

static void cmd_guserinfo(User* source, int ac, char** params) {
	dlink_node	*dl;
	User		*user;
	Metadata 	*md;
	Channel		*c;
	int			i;
	char		channels[512];

	if (!ac)
	{
		sendto_one (s_Guardian, source, "USERINFO requires a parameter");
		return;
	}
	if (!*params[0])
		return;
		
	if (!(user = find_user (params[0])))
	{
		sendto_one (s_Guardian, source, "Unknown user \002%s\002", params[0]);
		return;
	}

	channels[0]	= '\0';

	sendto_one (s_Guardian, source, "Information about \002%s\002 [%s]", user->nick, (HasId(user))? user->uid : "No UID");
	sendto_one (s_Guardian, source, " Userhost: %s@%s", user->user, user->host);
	sendto_one (s_Guardian, source, " IP Addr: %s",(user->ip)? user->ip : "No IP");
	sendto_one (s_Guardian, source, " Realname: %s", user->realname);
	sendto_one (s_Guardian, source, " Server: %s", user->serv->name);
	sendto_one (s_Guardian, source, " Version: %s", (user->version[0] != '\0')? user->version : "No Version Reply");
	sendto_one (s_Guardian, source, " Timestamp: %lu", (unsigned long) user->age);
	sendto_one (s_Guardian, source, " Modes: %s", create_usermode_string(user));
	sendto_one (s_Guardian, source, " Access Flags: %s", user->access);
	sendto_one (s_Guardian, source, " Metadata: ");
	
	DLINK_FOREACH (dl, user->metadata.head) {
		md = dl->data;
		sendto_one (s_Guardian, source, " - %s: %s", md->key, md->value);
	}
	
	DLINK_FOREACH (dl, user->channels.head)
	{
		c	= dl->data;

		if (channels[0] != '\0')
		{
			strlcat (channels, " ", sizeof(channels));
			strlcat (channels, c->name, sizeof(channels));
		}
		else
			strlcpy (channels, c->name, sizeof(channels));

		i++;

		if (i == 15)
		{
			sendto_one (s_Guardian, source, " Channels: %s", channels);
			channels[0] = '\0';
			i	= 0;
		}
	}

	if (i > 0)
		sendto_one (s_Guardian, source, " Channels: %s", channels);

	return;
}


/************************************************/
/**
 * Help functions
 */

static void help_guserinfo (User* u)
{
	sendto_one_help(s_Guardian,u, "USERINFO","Display all associated information for user.");
	return;
}

static void help_guserinfo_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002USERINFO [USER]\002");
    sendto_one(s_Guardian,u," ");
    sendto_one(s_Guardian,u,"     List all currently loaded modules, and their vendors.");
    sendto_one(s_Guardian,u," ");
    return;

}



