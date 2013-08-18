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
 *    $Id: parser.h 2204 2012-01-08 07:29:53Z jer $
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#define MAXPARA 15

//static const char *para[MAXPARA + 2];

char source[32];
char command[32];
char target[33];

int psr_init();
void psr_HandleEvent(Socket*,int);
void HandleClientCmds(User* u, int ac, char** av);
static void HandlePrivMsg(User*,int,char**);
static inline int break_params(int, char**,const char**,int,int);


#endif /*__PARSER_H__ */

