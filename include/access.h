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
 *    $Id: access.h 2273 2012-01-27 16:54:12Z twitch $
 */

 /******************************************/

#define ACC_FLAG_SUPER  "S"
#define ACC_FLAG_ADMIN  "A"
#define ACC_FLAG_USER   "u"
#define ACC_FLAG_OPER   "o"
#define ACC_FLAG_ACCESS "a"
#define ACC_FLAG_MODULE "m"
#define ACC_FLAG_LIST   "l" 
#define ACC_FLAG_JOIN   "j"
#define ACC_FLAG_SET    "s"
#define ACC_FLAG_DIE    "D"
#define ACC_FLAG_DEBUG  "d"
#define ACC_FLAG_REHASH "r"

#define ACC_ROOT "aAmljsSDdr"
#define ACC_ADMN "aAmljdr"
#define ACC_OPER "lj"
#define ACC_USER "u"

typedef struct access_ {

    char name[30];

    char cname[30]; 

    char flags[128];

    dlink_list hosts;

    int non_config;
} Access;


typedef struct acc_flags_ {
	char name[50];
	char flag[50];
}  AccessFlag;


int DestAccList();

int check_access		(User* u, char *level);
Access *new_access		(char *);
Access *access_exists	(char *);

int     set_access_flag (Access *,char *);
int		add_access_host (Access *,char *);


Access* find_access (User*);

Access* add_access(char *, char *, char *, int);
int del_access(char *);

void refresh_access (void);
int setAccess		(User* u);

AccessFlag *find_flag_byvalue (int);
AccessFlag *find_access_flag  (char *);

int 	add_access_flag 	  (char *, char *);
void 	init_access			  (void);
int 	del_acces_flag		  (char *);
void 	build_access_list();
dlink_list accesslist;
dlink_list access_flags;





