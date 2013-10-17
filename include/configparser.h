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
 *    $Id: configparse.h 2315 2012-01-30 03:29:51Z twitch $
 */


#ifndef __CONFPARSE_H__
#define __CONFPARSE_H__

#define CONF_REQUIRED 1
#define CONF_OPTIONAL 0

#define CONFIG_VERSION 2 
//use a prime number so we don't get collisions *cross our fingers* at least
#define HASH_BY 2579				
#define HASH_B 254


#define FOREACH_PARAM(ce) for (ce = ce->ce_entries; ce; ce = ce->ce_next)


int config_load(char *);
int config_parse(char *);
int destroy_config_tree();


typedef struct confbase_ {
	int entry_cnt;
	dlink_list entries;
	char map[255][2500]; 
} ConfBase;

typedef struct confentry_ {
	char key[1024];
	char value[1024];
} ConfEntry;


dlink_list config_tree[2580];

int config_tree_index[2580];
int config_tree_entries;

dlink_list*  get_config_base(char *);
char * get_config_entry(char *, char *);
char * get_config_entry_by_cb(ConfBase *, char *);


int get_config_bool(char *, char *, int);
int get_config_int(char *, char *, int);
int get_config_list(char *, char *, char, const char **);

#endif
