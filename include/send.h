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
 *    $Id: protocol.c 2182 2012-01-05 02:13:53Z  $
 */

#ifndef __SEND_H__
#define __SEND_H__


void send_cmd			(char*,char *,char *, ...);
void sendto_one			(User *, User *, char *, ...);
void sendto_one_privmsg	(User *, User *, char *, ...);
void sendto_one_notice		(User *, User *, char *, ...);
void sendto_one_numeric	(char *, User *, int, char *, ...);
void sendto_ircops			(User *, char *, ...);
void sendto_channel		(User *, Channel*, char*, ...);
void sendto_server			(User *,    char *, ...);
int 	sendto_socket			(Socket *, char *, ...);
void sendto_one_help		(User *, User *, char *, char *, ...);

#endif //__SEND_H__
