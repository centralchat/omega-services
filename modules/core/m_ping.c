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
 * Desc: Period checks to make sure we are still connected to primary server
 *
 *    $Id$
 *
 */

#include "stdinc.h"
#include "server.h"

/************************************************/
/**
 * Defines
 */

#define NAME "m_ping"
#define AUTHOR "Twitch"
#define VERSION "$Id $"


static int  ev_ping(int, void*);
static int  Module_Init  ();
static void Module_Close ();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7),Module_Init, Module_Close);

/*****************************************/
/**
 *  Module_Init() - Module factory called
 *                when the module is loaded.
 *   return int Any status changes the module should pass to
 *				the core... see MOD API documentation.
 */

static int Module_Init()
{     
 	 AddEventEx ("Uplink Ping", 120, EVENT_UNLIMITED, ev_ping);	 
     return MOD_CONT;
}



/**
 * Module_Close() 
 * Closing proceedure this is were you remove your clients
 * and commands and is called on unload.
 */
 
static void Module_Close()
{
	DelTimedEvent ("Uplink Ping");
	return;
}


static int ev_ping (int argc, void *args)
{
	char **argv = args;
	if (!servsock)
		return EVENT_CONT;
	if (servsock->sd == -1)
		return EVENT_CONT;
	if (!Uplink)
		return EVENT_CONT;
	if ((Uplink->lastpong - time(NULL)) > 120)
	{
		uplink_cleanup (NULL, 0);
		AddEventEx ("Connect Uplink", 60, 3, ev_connectuplink);
		return EVENT_CONT;
	}
    ircd_ping((IRCd.ts6)? Uplink->sid : Uplink->name);
	return EVENT_CONT;
}



