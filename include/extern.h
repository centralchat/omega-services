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
 *    $Id: extern.h 2204 2012-01-08 07:29:53Z jer $
 */


#ifndef __EXTERN_H__
#define __EXTERN_H__

#ifndef E
#	define E extern
#else
#	undef E
#	define E extern
#endif

E ServCmds* servcmds_list;   
E UserCmds* usercmds_list;          
E User* user_list;   
E Channel* chan_list; 
E Events* event_list;
E Link* serv_list;
E Module* module_list;

E int maxfds;

#endif /*__EXTERM_H__ */


