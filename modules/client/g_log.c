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
 *    $Id: g_log.c 2394 2012-06-30 15:56:22Z twitch $
 */
 

#include "stdinc.h"
#include "server.h"

 
#define NAME	"g_logging"
#define	AUTHOR	"Twitch"
#define VERSION	"$Id: g_log.c 2394 2012-06-30 15:56:22Z twitch $"

static int evh_glogcon    (int, void*);
static int evh_glognick   (int, void*);
static int evh_glogexit   (int, void*);
static int evh_glogjoin   (int, void*);
static int evh_glogpart   (int, void*);
static int evh_glogkick   (int, void*);
static int evn_glogrehash (int, void*);

int usercnt  = 0;
int maxusers = 0;

int Module_Open();
void Module_Close();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Open, Module_Close);

int Module_Open()
{
	int logusers = 0;
	//<logging channel="#Security" commands="yes" nicks="yes" channels="yes" users="yes">
	//Omega will still need to handle the Channel portion, as its vital for errors.
	logcmds = get_config_bool("logging", "commands", 0);
	logusers = get_config_bool("logging","users", 0);
	//logusers will imply both connects and nicks
	if (logusers) 
		lognicks = logclients = 1;
	else {
		lognicks = get_config_bool("logging","nicks", 0);
		logclients = get_config_bool("logging","connects", 0);
	}
	logchannels = get_config_bool("logging","channels", 0);
	AddEvent ("USERCONN", evh_glogcon);
	AddEvent ("USEREXIT", evh_glogexit);
	AddEvent ("USERNICK", evh_glognick);
	AddEvent ("CHANJOIN", evh_glogjoin);
	AddEvent ("CHANPART", evh_glogpart);
	AddEvent ("CHANKICK", evh_glogkick);
	AddEvent ("REHASH", evn_glogrehash);
	return MOD_CONT;
}

void Module_Close()
{
	//unset all LOGGING params cause we are bailing.
	logcmds = 0; lognicks = 0; logclients = 0; logchannels = 0;
	
	DelEvent ("USERCONN", evh_glogcon);
	DelEvent ("USEREXIT", evh_glogexit);
	DelEvent ("USERNICK", evh_glognick);
	DelEvent ("CHANJOIN", evh_glogjoin);
	DelEvent ("CHANPART", evh_glogpart);
	DelEvent ("CHANKICK", evh_glogkick);
	DelEvent ("REHASH", evn_glogrehash);
	

}
static int evn_glogrehash(int ac, void *nil) {
	
	int logusers = 0;
	
	sendto_logchan("\002Rehash:\002 Updating log parameters on rehash.");
	
	//unset all LOGGING params cause we are rehashing.
	logcmds = 0; lognicks = 0; logclients = 0; logchannels = 0;	
	 // <logging channel="#Security" commands="yes" nicks="yes" channels="yes" users="yes">
	 //Omega will still need to handle the Channel portion, as its vital for errors.
	logcmds = get_config_bool("logging", "commands", 0);
	logusers = get_config_bool("logging","users", 0);
	//logusers will imply both connects and nicks
	if (logusers) {
		lognicks = 1; 
		logclients = 1;
	} else {
		lognicks = get_config_bool("logging","nicks", 0);
		logclients = get_config_bool("logging","connects", 0);
	}
	logchannels = get_config_bool("logging","channels", 0);
	return EVENT_CONT;
}

static int evh_glogkick(int ac, void *echan) {
	evh_channel *echannel = echan;
	if (logchannels)
		sendto_logchan("\002Kick:\002 %s has been kicked from channel %s (%s)", echannel->u->nick, echannel->c->name, echannel->message);
	return EVENT_CONT;
}


static int evh_glogjoin(int ac,void *echan) {
	evh_channel *echannel = echan;
	if (logchannels)
	   	sendto_logchan("\002Join:\002 %s has joined channel %s", echannel->u->nick, echannel->c->name);
    return EVENT_CONT;
}
static int evh_glogpart(int ac,void *echan) {
	evh_channel *echannel = echan;
	if (logchannels)
		sendto_logchan("\002Part:\002 %s has left channel %s", echannel->u->nick, echannel->c->name);
	return EVENT_CONT;
}
static int evh_glognick(int ac, void *event_nick) {
	evh_nick *enick = event_nick;
	if (lognicks)
		sendto_logchan("\002Nick:\002 %s has changed his/her nickname to %s", enick->oldnick, enick->u->nick);
	return EVENT_CONT;
}

static int evh_glogexit(int ac, void *user) 
{
	User *u = user;
	usercnt--;
	if (logclients)
		sendto_logchan("\002Quit:\002 %s (%s) exiting from %s",u->nick,u->host, (u->serv)? u->serv->name : "Unknown");
	return EVENT_CONT;
}

static int evh_glogcon(int ac, void *user)
{
	User *u = user;
	if (sync_state != RUNNING)
		return EVENT_CONT;
			
	if (logclients) {
		if (usercnt > maxusers) 	{
			sendto_logchan ("\002Connect:\002 New user count high (Old %d) (New %d)", maxusers,usercnt);
			maxusers = usercnt;
		}
		
		if (u->ip) //since some of our protocols don't support IP yet, allow this to change format.
			sendto_logchan ("\002Connect:\002 %s (%s => %s) connecting from %s.", u->nick, u->ip, u->host, u->serv->name);
		else
			sendto_logchan ("\002Connect:\002 %s (%s) connecting from %s.", u->nick,u->host,u->serv->name);
	}
	alog(LOG_USER,"CONNECT: %s (%s) has connected from %s", u->nick, u->host, (u->serv)? u->serv->name : "Unknown");
	return EVENT_CONT;
}

