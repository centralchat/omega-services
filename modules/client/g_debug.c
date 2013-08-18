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
 *    $Id: g_debug.c 2366 2012-02-27 05:38:02Z twitch $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME	"g_debug"
#define VERSION "$Id: g_debug.c 2366 2012-02-27 05:38:02Z twitch $"
#define AUTHOR	"Jer"

extern dlink_list servcmds;
extern dlink_list usercmds;

extern dlink_list ServCmdTable[1024];
extern dlink_list UserCmdTable[1024];

dlink_list con_users;


static void cmd_chaninfo	(User *, int, char **);
static void cmd_chanlist	(User *, int, char **);
static void cmd_userinfo	(User *, int, char **);
static void cmd_userlist	(User *, int, char **);
static void cmd_seventlist  (User *, int, char **);
static void cmd_sendq		(User *, int, char **);
static void cmd_servlist	(User *, int, char **);
static void cmd_servcmds    (User *, int, char **);
static void cmd_usercmds    (User *, int, char **);
static void cmd_ucmdinfo    (User *, int, char **);
static void cmd_eventlist   (User *, int, char **);
static void cmd_fdlist		(User *, int, char **);
static void cmd_progstats	(User *, int, char **);
static void cmd_accessflags	(User *, int, char **);
static void cmd_hash		(User *, int, char **);
static void cmd_settings    (User *, int, char **);
static void cmd_conflood    (User *, int, char **);

static int Module_Init();
static void Module_Close();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, Module_Close);

int Module_Init()
{
	int ch;

#ifdef DEBUG_MODE
	ch = AddCmd(s_Guardian, "HASH",		ACC_FLAG_DEBUG,  cmd_hash, NULL, 1);
	ch = AddCmd(s_Guardian, "CHANINFO",	ACC_FLAG_DEBUG,  cmd_chaninfo, NULL, 1);
	ch = AddCmd(s_Guardian, "CHANLIST", 	ACC_FLAG_DEBUG,  cmd_chanlist, NULL, 1);
	ch = AddCmd(s_Guardian, "USERLIST",	ACC_FLAG_DEBUG,  cmd_userlist, NULL, 1);
	ch = AddCmd(s_Guardian, "SERVLIST", 	ACC_FLAG_DEBUG,  cmd_servlist, NULL, 1);
   	ch = AddCmd(s_Guardian, "EVENTLIST",  	ACC_FLAG_DEBUG,  cmd_eventlist, NULL, 1);
   	ch = AddCmd(s_Guardian, "SEVENTLIST", 	ACC_FLAG_DEBUG,  cmd_seventlist, NULL, 1);
	ch = AddCmd(s_Guardian, "SENDQ",	ACC_FLAG_DEBUG,  cmd_sendq, NULL, 1);
	ch = AddCmd(s_Guardian, "SERVCMDS", 	ACC_FLAG_DEBUG,  cmd_servcmds, NULL, 1);
       ch = AddCmd(s_Guardian, "CMDINFO", 	ACC_FLAG_DEBUG,  cmd_usercmds, NULL, 1);
	ch = AddCmd(s_Guardian, "UCMDINFO", 	ACC_FLAG_DEBUG,  cmd_ucmdinfo, NULL, 1);
	ch = AddCmd(s_Guardian, "FDLIST", 	ACC_FLAG_DEBUG,  cmd_fdlist, NULL, 1);
	ch = AddCmd(s_Guardian, "PROGSTATS",    ACC_FLAG_DEBUG,  cmd_progstats, NULL, 1);
	ch = AddCmd(s_Guardian, "ACCESSFLAGS",  ACC_FLAG_USER,  cmd_accessflags, NULL, 1);
	ch = AddCmd(s_Guardian, "CONFLOOD", 	ACC_FLAG_DEBUG, cmd_conflood, NULL, 1);
	ch = AddCmd(s_Guardian, "SETTINGS", 	ACC_FLAG_DEBUG, cmd_settings, NULL, 1);
#else
	throwModErr("This module requires debug to be enabled%s",".");
#endif
	return MOD_CONT;



}

void Module_Close()
{
	DelCmd(s_Guardian, "HASH",  cmd_hash, NULL);
	DelCmd(s_Guardian, "CHANINFO",  cmd_chaninfo, NULL);
	 DelCmd(s_Guardian, "CHANLIST",  cmd_chanlist, NULL);
	 DelCmd(s_Guardian, "CONFLOOD",  cmd_conflood, NULL);
	
	 DelCmd(s_Guardian, "USERLIST",  cmd_userlist, NULL);
	 DelCmd(s_Guardian, "SERVLIST",  cmd_servlist, NULL);
         DelCmd(s_Guardian, "EVENTLIST", cmd_eventlist, NULL);
	 DelCmd(s_Guardian, "SENDQ",     cmd_sendq, NULL);
	 DelCmd(s_Guardian, "FDLIST",    cmd_fdlist, NULL);
	 DelCmd(s_Guardian, "SEVENTLIST",cmd_seventlist, NULL);
	 DelCmd(s_Guardian, "SERVCMDS",  cmd_servcmds, NULL);
	 DelCmd(s_Guardian, "UCMDINFO",  cmd_ucmdinfo, NULL);
	 DelCmd(s_Guardian, "CMDINFO",   cmd_usercmds, NULL);
	 DelCmd(s_Guardian, "PROGSTATS", cmd_progstats, NULL);
	 DelCmd(s_Guardian, "ACCESSFLAGS", cmd_accessflags, NULL);

     	DelCmd(s_Guardian, "SETTINGS", cmd_settings, NULL);
        return;


}

/****************************************************/

static void cmd_hash (User *u, int ac, char **argv)
{
	int n, status;
	int i = 0;
	

    if (!*argv[0]) 
       return;
		
	i = HASH(argv[0],1024);
	sendto_one(s_Guardian,u,"%s hashes to %d", argv[0], i);
	return;
}


static void cmd_settings (User *u,int ac, char **argv)
{
    sendto_one (s_Guardian, u, "DataDir: %s", datadir);

    return;
}

static void cmd_accessflags (User *u,int ac, char **argv)
{
	dlink_node *dl;
	AccessFlag *al;
	
	DLINK_FOREACH(dl, access_flags.head)
	{
			al = dl->data;
			sendto_one(s_Guardian,u,"%s - %s",al->name,al->flag);
	
	}
	return;
}

static void cmd_progstats (User *u, int ac, char **argv)
{

	FILE * fp;
	char buf[400];
	char cmd[100];
    int ret;

	sendto_one(s_Guardian,u,"Program CPU Statistics");

	snprintf(cmd,(sizeof(cmd) - 1), "ps ux -p %lu | grep \'security\'",(unsigned long) getpid());
	fp = popen(cmd,"r");

	ret = fread(buf,1,sizeof(buf),fp);

	sendto_one(s_Guardian,u,"%s",buf);

	pclose(fp);

	return;
}


static void cmd_conflood (User *src, int ac, char **av) {
        User *u;
	dlink_node *dl, *tdl;
	
	char nick[255];
	char host[255];
	time_t end, start = time(NULL);
	
	int i = 0;
	
	    if (!*av[0]) { 
		sendto_one(s_Guardian, src, "Invalid Syntax: \002CONFLOOD [COUNT]\002"); 
		return;
	    }
	    int len = atoi(av[0]);
	    
	    for (i = 0; i < len; i++) {
	      snprintf(nick, sizeof(nick), "cli%d", i);
	      snprintf(host, sizeof(host), "cli%d.example.net", i);
	      
	      NewClient(nick, nick, guardian.host, nick);
	    
	      memset(nick, '\0', sizeof(nick));
	      memset(host, '\0', sizeof(host));
	    }
	    end = time(NULL);
	    sendto_one(s_Guardian,src,"Connected \002%d\002 clients.(In: \002%s\002)", i, timediff(end - start));

	return;
} 

static void cmd_chaninfo	(User *source, int ac, char **argv)
{
	dlink_node	*dl;
	char		*params[15];
	Channel		*c;
	struct ChanUser	*cu;
	int			cnt, i;
	char		nicklist[512];

	size_t nicklist_size = 0;

	i = 0;

	if (!*argv[0])
	{
		sendto_one (s_Guardian, source, "CHANINFO requires a parameter");
		return;
	}

	if (!(c = find_channel (argv[0])))
	{
		sendto_one (s_Guardian, source, "Unknown channel %s", argv[0]);
		return;
	}


	memset(nicklist,0,sizeof(nicklist));

	nicklist[0]	= '\0';

	sendto_one (s_Guardian, source, "Information about \002%s\002", c->name);
	sendto_one (s_Guardian, source, " Timestamp: %lu", (unsigned long) c->channelts);
	sendto_one (s_Guardian, source, " Modes: %s", create_chanmode_string(c));
	sendto_one (s_Guardian, source, " Users in channel: ");

	DLINK_FOREACH (dl, c->users.head)
	{
		cu	= dl->data;

		if (!cu->user)
			continue;

		if ((nicklist_size + strlen(cu->user->nick)) >= (sizeof(nicklist) - 1))
		{
			sendto_one(s_Guardian, source, "%s",nicklist);
			memset(nicklist,0,sizeof(nicklist));
			nicklist_size = 0;

			nicklist[0] = '\0';

		}

		if (nicklist[0] != '\0')
		{

			strlcat (nicklist, " ", sizeof(nicklist));
			strlcat (nicklist, cu->user->nick, sizeof(nicklist));
		}
		else
			strlcpy (nicklist, cu->user->nick, sizeof(nicklist));

		nicklist_size += strlen(cu->user->nick);

		i++;

	}

	if (i > 0)
		sendto_one (s_Guardian, source, " Users: %s", nicklist);

	return;
}


static void cmd_userinfo	(User *source, int ac, char **argv)
{
	dlink_node	*dl;
	User		*user;
	Channel		*c;
	int			i;
	char		channels[512];

	if (!argv)
	{
		sendto_one (s_Guardian, source, "USERINFO requires a parameter");
		return;
	}
	if (!*argv[0])
		return;
		
	if (!(user = find_user (argv[0])))
	{
		sendto_one (s_Guardian, source, "Unknown user \002%s\002", argv);
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

static void cmd_userlist	(User *source, int ac, char **argv)
{
	User		*user;
	int			i;
	dlink_node	*dl;

	i = 0;

	DLINK_FOREACH (dl, userlist.head)
	{
		user	= dl->data;

		i++;

		sendto_one (s_Guardian, source, "[%d] %s (%s@%s) connected to %s", i, user->nick, user->user,
					user->host, user->serv->name);
	}

	return;
}


static void cmd_chanlist	(User *source, int ac, char **argv)
{
	Channel		*c;
	dlink_node	*dl;
	int			i;

	i = 0;

	DLINK_FOREACH (dl, channels.head)
	{
		c	= dl->data;
		i++;

		sendto_one (s_Guardian, source, "[%d] %s containing %d user(s)", i, c->name, c->usercnt);
	}

	return;
}


static void cmd_sendq	(User *source, int ac, char **argv)
{
	struct sendq	*sq;
	dlink_node		*dl;
	int				i = 0;

	DLINK_FOREACH (dl, sendq.head)
	{
		sq	= dl->data;
		i++;

		sendto_one (s_Guardian, source, "[%d] buf: %s len: %d pos: %d", i, sq->buf, sq->len, sq->pos);
	}

	return;
}

static void cmd_servlist	(User *source, int ac, char **argv)
{
	Link		*li;
	dlink_node	*dl;
	int			i = 0;

	DLINK_FOREACH (dl, servlist.head)
	{
		li	= dl->data;

        i++;

		sendto_one (s_Guardian, source, "[%d] %s connected from %s", i, li->name, (li->uplink)? li->uplink->name : "");
	}

	return;
}

static void cmd_seventlist (User *u, int ac, char **av)
{
	return;
}


static void cmd_servcmds    (User *u, int ac, char **av)
{

	int i = 0;
	dlink_node *dl;
	ServCmds *sc;
        DLINK_FOREACH(dl,servcmds.head)
        {
			sc = dl->data;

			i++;
			sendto_one(s_Guardian,u,"[%d] (%s) %s",i,sc->name,(sc->module)? sc->module : "Core");

	}

}

static void cmd_ucmdinfo    (User *u, int ac, char **av)
{
	dlink_node *dl;
	int index = 0;
	int i = 0;
	UserCmds* uc;

	if (!*av[0])
	{
		sendto_one(s_Guardian,u,"This command requires a parameter");
		return;
	}

	index = HASH(av[0],1024);
	sendto_one(s_Guardian,u,"Listing USER issued command hooks for (%s)",av[0]);

	DLINK_FOREACH(dl,UserCmdTable[index].head)
	{
		uc = dl->data;

		i++;
		sendto_one(s_Guardian,u,"[%d] %s",i,uc->module);
	}

	return;
}
static void cmd_usercmds    (User *u, int ac, char **av)
{


	int i = 0;
	dlink_node *dl;
	struct cmdbase_ *cb;

	if (!*av[0])
			return;
			
	int index = HASH(av[0], CMD_HASH_KEY);
        CmdHash *ch = UserCmdHash[index];

	sendto_one(s_Guardian,u,"Listing command hooks for %s",av[0]);

        DLINK_FOREACH(dl,ch->commands.head)
        {
			cb = dl->data;

			i++;
			sendto_one(s_Guardian,u,"[%d] %s - %p", i, cb->name, cb->fnct);
	}

}

static void cmd_eventlist   (User *source, int ac, char **argv)
{
    dlink_node  *dl;
    Events      *ev;
    int         i = 0;

    DLINK_FOREACH (dl, events.head)
    {
        ev  = dl->data;

        i++;

        if (ev->interval == -1)
            sendto_one (s_Guardian, source, "[%d] (%s)", i, ev->name);
        else
            sendto_one (s_Guardian, source, "[%d] (%s) [Next run: %d sec(s)]", i, ev->name, (ev->when - ServerTime));
    }

    if (i <= 0)
        sendto_one (s_Guardian, source, "No events");

    return;
}

static void cmd_fdlist	(User *source, int ac, char **argv)
{
	dlink_node		*dl;
	Socket			*s;
	char			type[512], mode[512];

	DLINK_FOREACH (dl, sockets.head)
	{
		s	= dl->data;

		if ((s->flags & SOCK_UPLINK))
			strlcpy (type, "Uplink", sizeof(type));
		else if ((s->flags & SOCK_DCC))
			strlcpy (type, "DCC", sizeof(type));
		else
			strlcpy (type, "OTHER", sizeof(type));

		if ((s->flags & SOCK_READ))
			strlcpy (mode, "Read", sizeof(mode));
		else if ((s->flags & SOCK_WRITE))
			strlcpy (mode, "Write", sizeof(mode));
		else
			strlcpy (mode, "Unknown", sizeof(mode));

		sendto_one (s_Guardian, source, "[%s] [FD: %d] [Type: %s] (%s) (%s)", (s->name[0] != '\0')? s->name : "", s->sd, type, (s->flags & SOCK_CONNECTING)? "Connecting" : "Connected", mode);
	}

	return;
}




