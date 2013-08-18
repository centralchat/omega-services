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
 *    $Id: g_core.c 2376 2012-03-03 22:34:49Z twitch $
 */


#include "stdinc.h"
#include "server.h"




#define NAME "g_core"
#define AUTHOR "$Author: twitch $"
#define VERSION "$Id: g_core.c 2376 2012-03-03 22:34:49Z twitch $"


extern Link* serv_list;
extern User* user_list;
extern Channel* chan_list;
extern Module* module_list;

static int guardian_load();
static void guardian_unload();

static void my_privmsg(User*, int, char**);
static void Guardian_Init();




/***********************************************/

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),guardian_load, NULL);


static int guardian_load()
{
      Guardian_Init();
      AddUserCmd("PRIVMSG", my_privmsg);
      return MOD_CONT;
}


void guardian_unload()
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

static void Guardian_Init()
{
	User    *u = NULL;
	Channel	*c = NULL;
	char	*ce = NULL, *chan = NULL;
	int     n = 0, cnt = 0;

	const char *list[15];

	char chan_tmp[1024];
	

	strcpy (guardian.nick,	"Guardian");
	strcpy (guardian.host,	"network.tld");
	strcpy (guardian.user,	"security");
	
	if ((ce = (char *)get_config_entry("guardian", "nick")))	
		strlcpy (guardian.nick,	ce, sizeof(guardian.nick));
	if ((ce = (char *)get_config_entry("guardian", "host")))
		strlcpy (guardian.host,	ce, sizeof(guardian.host));
	if ((ce = (char *)get_config_entry("guardian", "ident")))
		strlcpy(guardian.user,ce, sizeof(guardian.user));
	
	strcpy (guardian.desc,	"Network Security Service");
	u = NewClient(guardian.nick, guardian.user, guardian.host, guardian.desc);
	if (!u) {
		alog(2,"Unable to assign guardian user...");
		Exit(0);
	}
	
	s_Guardian  = u;
	s_Guardian->fantasy  = 1;

	if (!(c = find_channel (logchan)))
	{
		c		= new_chan (logchan);
		c->channelts	= time(NULL);
	}

	if (!in_channel (s_Guardian, c))
			AddToChannelU (s_Guardian, c);

	for (n = 0; n < 15; n++)
		list[n] = NULL;

	if ((cnt = get_config_list("guardian", "channels", ',',list))) {
		//printf("Recieved: %d channels to join\n", cnt);
		for (n = 0; n < cnt; n++) {
			if (!list[n])
				continue;
			//printf("Adding g_core channel for: [%d] %s\n", n, (char*)list[n]);			
			if (!(c = find_channel ((char *)list[n]))) {
				c		= new_chan ((char*)list[n]);
				c->channelts	= time(NULL);
			}
			if (!in_channel (s_Guardian, c))
				AddToChannelU (s_Guardian, c);

			//if ((chan = list[cnt]))
			//free(list);
		 }
	}
    	return;
}

/***********************************************/




