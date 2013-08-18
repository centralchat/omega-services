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
 *    $Id: tools.h 2278 2012-01-27 23:01:13Z twitch $
 */

#ifndef __TOOLS_H__
#define __TOOLS_H__

#define SWP(x,y) (x^=y, y^=x, x^=y)

    size_t strlcpy (char *, const char *, size_t);
    size_t strlcat (char *, const char *, size_t);

    extern char p10_id[3];

    char * generate_uid (void);
    void   generate_p10_uid (User *);

    void ExpandParv(char *, int, int, int, char **);

    int generictoken(char, int, char *, char **);

    uint32 hash(char *item);

    unsigned int hash_safe(char *item, ub4 len);

    char *timediff(time_t);

    char *str_signed(unsigned char *str);

    void ntoa(struct in_addr addr, char *ipaddr, int len);

    char *str_ntoa(struct in_addr addr);

    char * strtolower (char *);

    void reverse_ip(const char *, char *);

#endif

