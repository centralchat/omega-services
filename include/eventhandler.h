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
 *    $Id: eventhandler.h 2395 2012-06-30 16:02:11Z twitch $
 */


#ifndef __EVENTHANDLER_H__
#define __EVENTHANDLER_H__


#define HASH(x,y) hash_safe(x,y)

#define EVENT_UNLIMITED -1

/****************************************/
/**
  * Socket/Select Events
  */


enum EventStatus {
    EVENT_OK, 
    EVENT_CONT, 
    EVENT_STOP, 
    EVENT_ABORT
};

enum EventType {
    EVENT_READ =  0,
    EVENT_WRITE = 1,
    EVENT_ERROR = 2,
    EVENT_CONNECT = 3,
    EVENT_ENDBURST = 4,
    EVENT_NICK = 5,
    EVENT_QUIT = 6,
    EVENT_VERSION = 7,
    EVENT_SERVERCONN = 8,
    EVENT_SERVEREXIT = 9,
};


typedef void cmd_fnct(int, char**);


typedef struct servcmd_ {

	char name[40];
    char* module;

	void (*fnct)(Link*,int,char**);

	struct servcmd_  *next,*prev;

} ServCmds;



typedef struct usercmd_ {

       char* module;

       int active;      //if our command is active

	void (*routine)(User*,int,char**);

	struct usercmd_  *next,*prev;

} UserCmds;


typedef struct
{
	Channel		*c;
	User		*u;
	char        message[2048];
} evh_channel;

typedef struct
{
	User *u;
	char oldnick[32];
} evh_nick;

/********************************************/



dlink_list UserCmdTable[1024];
dlink_list ServCmdTable[1024];
dlink_list sockevents;
dlink_list err_handlers;


/*********************************************/

typedef void HandleEvent (int);



/*********************************************/
/**
 * Server Events
 */

typedef struct EventBase
{
    time_t  age;

    char *module; //module the event is associated with.

    char name[32];

    int (*handler)(int,void *);

    int recall; //if the event can only be called once :)

    int called; //how many times each event was called.

    time_t  when;
	
    int     interval;
	
	time_t last; //when we were last called.

} Events;

/****************************************/

HandleEvent* EventList[1024];


void DelEvent(char *event, void* fnct);


#define AddEvent(name,fnct) AddEventEx(name, -1, -1, fnct)
#define AddTimedEvent(event, fnct, interval) AddEventEx(event, interval, -1, fnct)
#define DelTimedEvent(event) DelEvent(event, NULL)
#define Event(event, ac, av) EventEx(event, ac, av, NULL);

int AddEventEx (char *, int, int, int (*)(int,  void *));
void EventEx   (char *, int, void *, int (*)(char*, Events*, int, void*));


void AddServCmd       (char *,void (*fnct)(Link*,int,char**));
int  HandleServCmd    (char *, Link*, int , char **);
void DelServCmd        ();


ServCmds* FindServCmd (char *cmd);

//error stuff
void catchCoreErr (int errcode, char *errstr);
void AddErrHandler (int errcode, void *fnct);

void AddUserCmd       (char *, void (*fnct)(User*,int,char**));
int  HandleUserCmd    (char *, User*, int , char **);
void DelUserCmd       (char *,void (*fnct)(User*,int,char**));

/************************************/

void HandleMsg       (char *msgtype,char *source, char *dest, char *msg);
void AddModuleMessage(char *msg, void (*fnct) (char *src,char *dest,char *msg));

dlink_list  events;

void RunTimedEvents   ();
void destroy_event_list();

int ev_housekeeping	(int,  void*);
int ev_connectuplink (int, void *);
#endif


