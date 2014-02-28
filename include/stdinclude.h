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
 *    $Id: stdinc.h 2204 2012-01-08 07:29:53Z jer $
 */

#include "setup.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>	/* for umask() on some systems */
#include <sys/types.h>
#include <sys/time.h>
#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h> /* for rlimit */
#endif
#include <fcntl.h>
#include <dlfcn.h>
#include <math.h>
#include <pthread.h>

#ifdef HAVE_SYS_EVENT_H
# include <sys/event.h>
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include <assert.h>

#include <dirent.h>
#include <strings.h>
#include <sys/select.h>
#include <ctype.h>

#ifdef USE_DMALLOC
# include <dmalloc.h>
#endif

#ifdef HAVE_SQLITE
# include <sqlite3.h>
#endif

#ifdef HAVE_GNUTLS
# include <gnutls/gnutls.h>
#endif

#include <sys/un.h>


#ifndef va_copy
# ifdef __va_copy
#  define VA_COPY(DEST,SRC) __va_copy((DEST),(SRC))
# else
#  define VA_COPY(DEST, SRC) memcpy ((&DEST), (&SRC), sizeof(va_list))
# endif
#else
# ifdef HAVE_VA_LIST_AS_ARRAY
#   define VA_COPY(DEST,SRC) (*(DEST) = *(SRC))
# else
#   define VA_COPY(DEST, SRC) va_copy(DEST, SRC)
# endif
#endif 

#if INTTYPE_WORKAROUND
# undef int16
# undef int32
#endif

#ifdef USE_KQUEUE
# ifndef HAVE_KEVENT
#  undef USE_KQUEUE
# endif
#endif

#ifdef USE_KQUEUE
# error "Kqueue support is currently suspended awaiting better support"
#endif

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef ub4
	typedef  unsigned long  int  ub4; 
#endif

#ifndef TRUE
#	define TRUE 1
#endif 
#ifndef FALSE
#	define FALSE 0
#endif 

#ifndef strlcpy
#	define strlcpy strncpy
#endif

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t   uint8;



#include "dlink.h"
