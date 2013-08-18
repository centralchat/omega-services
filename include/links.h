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
 *    $Id: links.h 2394 2012-06-30 15:56:22Z twitch $
 */

#ifndef _LINKS_H_
#define _LINKS_H_


Link* find_serv_num  (int);
Link* find_serv      (char *);
//Link* new_serv       (char*, int);
Link	*new_serv	(Link *, char *);
void exit_serv       (char *);
int remain_eos       (void);
int link_count       (void);
void  update_eos     (char*);
void exit_serv_linked(char *);

int burst_local_servers    (int, void*);

#endif


