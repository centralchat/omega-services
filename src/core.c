/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-10 core Dev Team
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
 *    $Id: core.c 2252 2012-01-22 09:21:23Z twitch $
 */

#include "stdinc.h"
#include "server.h"



Core* Core_Construct() 
{
	Core *core;
	
	if (!(core = (Core*) malloc(sizeof(Core))))
			return NULL; //we cant init our core struct
			
	core->age = time(NULL);
	sync_state = core->state = STARTUP;
	core->exit = Exit;
	core->rehash = Rehash;
	core->log = alog;
	//memset(core->cm,'\0',sizeof(core->cm));
	return core;
}



