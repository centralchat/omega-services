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
 *    $Id: protocol.c 2310 2012-01-30 01:59:42Z jer $
 */

#include "stdinc.h"
#include "server.h"

extern Link* serv_list;

IRCDProto IRCd;


int protocol_init()
{


     /*
	  ** lets start with a clean struct so if something
	  ** is null we know its not supported :) Twitch
	  */

	IRCd.ircd_connect	= ircd_null;
	IRCd.ircd_ping		= ircd_null;
	IRCd.ircd_add_user	= ircd_null;
	IRCd.ircd_oper		= ircd_null;
	IRCd.ircd_wallop	= ircd_null;
	IRCd.ircd_xline		= ircd_null;
	IRCd.ircd_version	= ircd_null;
	IRCd.ircd_set_cmode = ircd_null;
	IRCd.ircd_join		= ircd_null;
   	IRCd.ircd_part    	= ircd_null;
	IRCd.ircd_mode 	= ircd_null;
	IRCd.ircd_umode 	= ircd_null;
	IRCd.ircd_kill		= ircd_null;
	IRCd.ircd_numeric	= ircd_null;
 	IRCd.ircd_server   	= ircd_null;
	IRCd.push		= 0;
    	IRCd.ts          		= 0;
    	IRCd.ts6           	= 0;
   	IRCd.p10       		= 0;

	return 0;
}



/*******************************************/


void add_protocol(char* name,char *version,char *author, int ts6)
{
	memcpy(IRCd.name,name,sizeof(IRCd.name));
	memcpy(IRCd.version,version,sizeof(IRCd.version));
	memcpy(IRCd.author,author,sizeof(IRCd.author));
	IRCd.age = time(NULL);

      if (ts6 > 0)
      {
        IRCd.require_ts6    = 1;
        IRCd.ts             = 1;
        IRCd.ts6            = 1;
      }
      else
	 IRCd.ts6 = 0; //our protocol doesn't support TS6 override config settings.

	if (skip_banner)
		return;

	alog(0,"Loaded Protocol: \033[1;32m%s\033[0m",name);

	if (!debug ||!nofork)
		printf("Loaded Protocol: \033[1;32m%s\033[0m\n",name);

	if (ts6 > 0) {
		alog(0,"Enabled TS6 Support\n");
		if (!debug ||!nofork)
			printf("Enabled TS6 Support\n");
	}
}

void addp_connect(void (*func) ())
{
     IRCd.ircd_connect = func;
}

void addp_oper(void (*func)(char *source, char *opertype))
{
	IRCd.ircd_oper = func;
}

void addp_add_user(void (*func)(User* user))
{
	IRCd.ircd_add_user = func;
}

void addp_ping(void (*func)(char *source))
{
 	IRCd.ircd_ping = func;
}

void addp_wallop(void (*func)(char *source,char *msg))
{
	IRCd.ircd_wallop = func;
}

void addp_xline(void (*func)(char *xline,char *source,char *mask, int time, char *reason))
{
	IRCd.ircd_xline = func;
}

void addp_version(void (*func)(User* user))
{
	IRCd.ircd_version = func;
}

void addp_set_cmode(void (*func)(Channel*,int,char *))
{
	IRCd.ircd_set_cmode = func;
}

void addp_set_umode(void (*func)(User*,int,char *))
{
	IRCd.ircd_set_umode = func;
}

void addp_join(void (*func)(User*,Channel*))
{
	IRCd.ircd_join = func;
}

void addp_part(void (*func)(User*,Channel*,char*))
{
    IRCd.ircd_part = func;
}

void addp_mode(void (*func)(User*,Channel*,char*))
{
        IRCd.ircd_mode = func;
}

void addp_umode(void (*func)(User*,char*))
{
        IRCd.ircd_umode = func;
}

void addp_kill (void (*func)(User *, char *))
{
	IRCd.ircd_kill	= func;
}

void addp_server (void (*func)(Link *))
{
    IRCd.ircd_server    = func;
}

void addp_numeric	 (void (*func)(User*, int, char *))
{
	IRCd.ircd_numeric = func;
}


/*******************************************/

void ircd_mode(User* u,Channel* c,char *fmt, ...)
{
	char tmp[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp, 511, fmt, args);
        IRCd.ircd_mode(u,c, tmp);
   	va_end(args);
 }

/*******************************************/

void ircd_umode(User* u,char *mode)
{

        IRCd.ircd_umode(u,mode);
}


/*******************************************/

/* Assign this to avoid cores when functions are not defined in the protocol */
void ircd_null()
{
	return;
}

void ircd_numeric(User *u, int numeric, char *fmt, ...) {
	char *tmp = (char *) malloc(sizeof(char) * 512);
	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp,512, fmt, args);
	IRCd.ircd_numeric(u, numeric, tmp);
    va_end(args);
	free(tmp);
}

void ircd_xline(char *xline, char *source, char *mask, int time, char *reason, ...)
{

    char *tmp = (char *) malloc(sizeof(char) * 512);
	va_list args;

	va_start(args,reason);
	vsnprintf(tmp,512,reason,args);
	IRCd.ircd_xline(xline,source,mask, time, tmp);


    va_end(args);
	free(tmp);
}

void ircd_set_push() { IRCd.push = 1; }
void ircd_version (User* user)
{
	IRCd.ircd_version (user);
}

void ircd_connect()
{
    IRCd.ircd_connect();
}

void ircd_server(Link *serv)
{
    IRCd.ircd_server(serv);
}

/*void ircd_add_user(char *source,char *ident, char *host,char *modes, char *name)*/
void ircd_add_user(User* user)
{
    IRCd.ircd_add_user(user);
}

void ircd_ping(char *source)
{
	IRCd.ircd_ping(source);
}

void ircd_oper(char *source, char *opertype)
{
	IRCd.ircd_oper(source,opertype);
}

void ircd_wallop(char *source,char *msg, ...)
{
        char *tmp = (char *) malloc(sizeof(char) * 512);
	va_list args;

	va_start(args,msg);
	vsnprintf(tmp,512,msg,args);

	if (source == NULL)
		IRCd.ircd_wallop (CfgSettings.servername, tmp);
	else
        IRCd.ircd_wallop (source, tmp);

        va_end(args);
	free(tmp);
}

void ircd_set_cmode(Channel* u,int argc,char *modes)
{
	IRCd.ircd_set_cmode(u,argc,modes);
}

void ircd_set_umode(User* u,int argc,char *modes)
{
	IRCd.ircd_set_umode(u,argc,modes);
}


void ircd_join(User* u,Channel* c)
{
	IRCd.ircd_join(u,c);
}

void ircd_part(User *u, Channel *c, char *message)
{
    IRCd.ircd_part(u,c,message);
}

void ircd_kill (User *u, char *message)
{
	IRCd.ircd_kill (u, message);
}

/*****************************************/
