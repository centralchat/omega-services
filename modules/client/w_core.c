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
 *    $Id: w_core.c 2201 2012-01-08 07:08:09Z jer $
 */


#include "stdinc.h"
#include "server.h"

#define NAME "w_core"
#define AUTHOR "$Author: twitch $"
#define VERSION "$Id: w_core.c 2201 2012-01-08 07:08:09Z jer $"


extern Link* serv_list;
extern User* user_list;
extern Channel* chan_list;
extern Module* module_list;

static int watch_load();
static void watch_unload();

static void my_privmsg(User*, int, char**);
static void watch_Init();

struct watchman_ {
  char nick[30];
  char name[64];
  char ident[30];
  char host[100]; 

} Watchman;


/***********************************************/

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,7,4),watch_load, NULL);


static int watch_load()
{
      watch_Init();
      AddUserCmd("PRIVMSG", my_privmsg);
      return MOD_CONT;
}


void watch_unload()
{
	exit(0);
}



/***********************************************/


static void my_privmsg(User* u,int argc, char **msg)
{


	if (*msg[2] != '\001')
		return;

	if (strcasecmp(msg[2],"\001VERSION\001")==0)
	{
		send_line(":%s NOTICE %s :\001VERSION Omega(%s)-%s\001",s_Guardian->nick,u->nick,PACKAGE_VERSION,VERSION_STRING);
		return;
	}

      return;

}

/***********************************************/

static void watch_Init()
{
	User    *u;
	Channel	*c;
	char	*chan;
	int n, cnt;
	dlink_node	*dl, *tdl;
	ConfBase *cb;
	
	char *ce;
	char *start;
	char chan_tmp[1024];
	

	//<watchman name="Guardian" host="network.tld" ident="security" channels="#Omega">
        strlcpy(Watchman.nick, "Watchman", sizeof(Watchman.nick));
        strlcpy(Watchman.host, "network.tld", sizeof(Watchman.host));
        strlcpy(Watchman.ident, "security", sizeof(Watchman.ident));
	
	if ((ce = (char *)get_config_entry("watchman", "nick")))	
		strlcpy (Watchman.nick,	ce, sizeof(Watchman.nick));
	if ((ce = (char *)get_config_entry("watchman", "host")))
		strlcpy (Watchman.host,	ce, sizeof(Watchman.host));
	if ((ce = (char *)get_config_entry("watchman", "ident")))
		strlcpy(Watchman.ident,ce, sizeof(Watchman.ident));
	
	    u = NewClient(Watchman.nick, Watchman.ident, Watchman.host, "Security Reporting Client");
	    if (!u)
	    {
		 alog(2,"Unable to assign guardian user...");
	   	 Exit(0);
	    }


	if (!(c = find_channel (logchan)))
	{
		c		= new_chan (logchan);
		c->channelts	= time(NULL);
	}

	if (!in_channel (u, c))
	AddToChannelU (u, c);

	memset(chan_tmp, 0, sizeof(chan_tmp));
	
	if ((ce = (char *)get_config_entry("watchman", "channels")))
	{
			for (; *ce; ce++)
			{
				start = ce;
				while ((*ce != ',') && (*ce))
						ce++;
				strncpy(chan_tmp, start, ce - start);
				if (!(c = find_channel (chan_tmp)))
				{
						c		= new_chan (chan_tmp);
						c->channelts	= time(NULL);
				}

				if (!in_channel (u, c))
						AddToChannelU (u, c);
						
				memset(chan_tmp,0,56);
			}
	}
    return;

}

/***********************************************/




