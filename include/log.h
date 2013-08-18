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
 *    $Id: log.h 2204 2012-01-08 07:29:53Z jer $
 */

#ifndef __MYLOG_H__
#define __MYLOG_H__

int uselogchan;

static FILE *logfd;

void open_log    (void);
void close_logs	(void);
void Log        (char *, ...);
void alog       (int, char*, ...);

void sendto_logchan (char *, ...);
void sendto_console (char *, ...);

#define LOG_INFO    0

/**
 * Debug  log types
 */

#define DEBUG    1
#define DEBUG_2  2
#define DEBUG3    11

#define LOG_DEBUG   1
#define LOG_DEBUG2  2
#define LOG_DEBUG3  11

/**
 * Errors
 */

#define LOG_ERROR 3
#define LOG_FATAL 4


/**
 * Standard log types
 */

#define LOG_EVENT   5
#define LOG_LINK    6
#define LOG_USER    7
#define LOG_CHAN    8
#define LOG_IO      9
#define LOG_MODULE 10
#define LOG_SOCKET 13

/**
 * Backtracing
 */

#define LOG_BACKTRACE 12

void CleanUpLogs(int,char**);


#endif //__LOG_H__

