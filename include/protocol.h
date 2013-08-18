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
 *    $Id: protocol.h 2310 2012-01-30 01:59:42Z jer $
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


typedef struct PROTOCOL {

   char name[50];
   char author[50];
   char version[100];

   time_t age;

   void (*ircd_connect)  ();
   void (*ircd_oper)     (char *source, char *opertype);
   void (*ircd_add_user) (User *user);
   void (*ircd_ping)     (char *source);
   void (*ircd_wallop)   (char *source, char *msg);
   void (*ircd_xline)    (char *xline, char *source, char *mask, int time, char *reason);
   void (*ircd_version)  (User* user);
   void (*ircd_set_umode) (User* u,int argc, char* modes);
   void (*ircd_set_cmode) (Channel* c,int argc, char* modes);
   void (*ircd_join)      (User*,Channel*);
   void (*ircd_part)      (User*,Channel*,char*);
   void (*ircd_mode)      (User*,Channel*,char*);
   void (*ircd_umode)     (User*,char*);
   void (*ircd_kill)	  (User *, char *);
   void (*ircd_server)    (Link *);
   void (*ircd_numeric)   (User*, int, char*);

   CMode* cmodes;
   int  push; //InspIRCd numeric uses push - enable this support
   int	ts;
   int	ts6;
   int  require_ts6;
   int  use_push;

   int  p10;

} IRCDProto;


int protocol_init ();


int protocol_open (char *filename);
void add_protocol (char* name,char *version,char *author, int ts6);


void addu_modes      (int mode, uint32 bits, uint32 flags);
void addp_connect    (void (*func) ());
void addp_oper       (void (*func)(char *source, char *opertype));
void addp_add_user   (void (*func)(User*));
void addp_ping       (void (*func)(char *source));
void addp_wallop     (void (*func)(char *source,char *msg));
void addp_xline      (void (*func)(char *xline,char *source,char *mask, int time, char *reason));
void addp_version	(void (*func)(User*));
void addp_join       (void (*func)(User*,Channel*));
void addp_part       (void (*func)(User*,Channel*,char*));
void addp_set_cmod   (void (*func)(Channel*,int,char *));
void addp_set_umode  (void (*func)(User*,int,char *));
void addp_umode (void (*func)(User*,char*));
void addp_mode  (void (*func)(User*,Channel*,char*));
void addp_server     (void (*func)(Link *));
void addp_numeric	 (void (*func)(User*, int, char *));


void ircd_connect();

void ircd_add_user  (User*);
void ircd_ping      (char *source);
void ircd_oper      (char *source, char *opertype);
void ircd_wallop    (char *source,char *msg, ...);
void ircd_xline     (char *xline, char *source, char *mask, int time, char *reason, ...);
void ircd_version   (User*);
void ircd_join      (User*,Channel*);
void ircd_set_cmode (Channel* c,int argc,char *modes);
void ircd_set_umode (User* u,int argc,char *modes);
void ircd_mode      (User*,Channel*,char*, ...);
void ircd_umode     (User*,char*);
void ircd_kill		(User *, char *);
void ircd_server    (Link *);
void ircd_numeric	(User *, int, char *, ...);



//UMode *ircd_umodes  ();
void ircd_null      (); //our null function :)

extern char *ircd_umodes[128];
extern IRCDProto IRCd;

#endif    /*__PROTOCOL_H__ */


