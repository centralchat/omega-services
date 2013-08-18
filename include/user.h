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
 *   but WITHOUT ANY WARRANTY; without even the implied warrantyof
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *    $Id: user.h 2394 2012-06-30 15:56:22Z twitch $
 */

#ifndef __USER_H__
#define __USER_H__

#define HasId(x)   		((x->uid[0] != '\0')? 1 : 0)
#define MyConnect(x)	       ((x->myconnect == 1)? 1 : 0)

dlink_list	userlist;

typedef struct USER User;
typedef void help_cmd(User*);

struct USER {
    time_t age;         //TS for the user structure :)
    char nick[50];
    char host[64];      // Users actual hostname
    char virthost[63];  // Users virtual host
    char user[10];      // Users ident
    char realname[50];  // Users realname
    char ip[46];        // Users IP

    char	version[512];	// Users CTCP VERSION reply

    Link    *serv;      // Server the user is found on

	char uid[15];       // Users UID (used for TS6)

	int registered;     // using a registered nickname
	int oper;           // is an IRC Operator
	int ssl;            // is using a secure connection
	int bot;            // is a bot (not supported for all IRCds)
	int service;        // has service protection or is a network service
	int webtv;          // WebTV will make us use privmsg when we get to it ;)
	int cloak;          // Has a cloaked host
	int vhost;          // has a vhost

	int invis;          // is invisible
	int priv;           // has PRIVATE mode set hides channels to users
	int admin;	    // user is a Server Admin
	int myconnect;
    int hideoper;
	int fantasy; 		//Handle Fantasy commands.
	time_t	lastsent;

	int lines;
	int ignore;

    char  access[128]; 	    //Access FLAGS

	dlink_list  metadata;
	dlink_list  channels;

};

User* AddUser  (char*,char*,char*,char*,char*,char *,char *, char *, char *);
User* find_user (char*);
User* find_uid  (char *);
User *new_user  (char *);
int AddUserIp	(User*, char *, int);

void exit_user  (char *);

void exit_one_user      (User *, char *);
void exit_local_users	(char *);
void exit_remote_users	(void);
int burst_local_users	(int, void *);
int introduce_users (int, void*);

User* NewClient  (char*,char*,char*,char*); //add a service to the network.
int DelClient    (char *);    	        	//remove a service from the network.

void NewNick      (User *,char *);
User* isOurClient (char *);                  //returns if the client is on our server
int CheckIgnore   (User *);

char * create_usermode_string   (User *);
void read_usermode_string       (User *, char *);

struct guardian {
    char    nick[30];
    char    host[64];
    char    desc[50];
    char    user[10];
	dlink_list	chanlist;
} guardian;

User* s_Guardian;
User* s_Scanner;

int logclients;
int lognicks;

char *Guardian;

#endif //__USER_H__
