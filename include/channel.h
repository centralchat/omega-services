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
 *    $Id: channel.h 2220 2012-01-15 03:03:09Z twitch $
 */


#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#define MAX_CHAN 34
#define KEYLEN   15

dlink_list  channels;


/***********************************************/

typedef struct channel_ {

	char 	name[MAX_CHAN];
	char 	key[KEYLEN];
	char	forward[MAX_CHAN];
	char	unknownmodes[32];
        char    modef[MAX_CHAN]; 
	int    ulimit;
	
	int joincnt;     //join count - This is only used to detect join/p floods
	time_t lastjoin; //last channel join
	int jflood;      //has join flooding been triggered?

	
       time_t channelts;
	
	/* the number of users in the channel 
	  * this is used to check weather or not
	  * we want to destroy the channel in our list 
	  */
	int usercnt; 
			   
	//channel flags
	int registered;     //registered channel tho we really don't need to know this yet we may need in the future.
	int ssl;            //we should respect this mode channel is SSL only
	int priv;           //channel is set to private and not viewable in the list
	int flood;          //flood protection mode
	int join;           //join protection mode
	int regonly;        //Registered nicks only can join
	int limit;          //channel has a limit
	int linked;         //channel has a link setup
	int operonly;       //channel is oper only (This isn't supported by all IRCds but we must know this)

	int onlyoptopic;	// Only channel operators may change the topic
	int external;		// No external messages accepted for this channel
    
	//we don't really need to respect these modes but be leery of them.
	int haskey; //if the channel has a key set
	
	int invite; //channel is invite only
	
	int audit; //Stupid unreal mode but if we ever use /names we need to know that Channel Ops are only gonna show.

       dlink_list  users;
       dlink_list  metadata;	
       
} Channel;

struct ChanUser {
    User    *user;
    int     owner;
    int     protect;
    int     op;
    int     halfop;
    int     voice;
};

struct CMode {
    char    *character;
    int     setparam;
    int     unsetparam;
    char    *symbol;
};

/***********************************************/


Channel* find_channel (char *chan);
Channel* new_chan     (char *chan);
void dest_chan        (char *chan);

extern struct CMode ircd_cmodes[128];

typedef struct ChanUser ChanUser;

char * create_chanmode_string (Channel *);

struct ChanUser *new_chanuser (User *);
struct ChanUser *find_chanuser (User *, Channel *);

void AddToChannel   (char *, Channel *);
void AddToChannelU  (User *, Channel *);
void DelFromChannel (User *, Channel *);
void KickFromChannel(User *, Channel *, char *);

int findcmode_bysymbol (char *);

int in_channel	(User *, Channel *);

#endif //__CHANNEL_H__


/* vim: set tabstop=4 shiftwidth=4 expandtab */

