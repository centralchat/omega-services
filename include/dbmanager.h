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


#ifndef __DBMANAGER__H__
#define __DBMANAGER__H__

//these will FIRE off DB events.
enum DBImplements {
	DBI_START,
		DBI_OPEN, DBI_LOAD, DBI_QUERY,
		DBI_UPDATE, DBI_DROP, DBI_SAVE,
		DBI_INSERT,	DBI_CLOSE,
	DBI_END
};

//allow for easy addition of types
enum DBTypes {
	DBT_STAR,
		DBT_STRING, DBT_INTEGER, DBT_FLOAT,
	DBT_END
};

typedef int (*DBFnct)();
DBFnct db_handlers[DBI_END];

/**
 * Conceptual DB Structure 
 * @note This may change as i decide how to best implement this
 * 
 * [R_NAME] [KEY] [VALUE]
 *  AC Twitch amljsSDdr
 *  AH Twitch twitch@* *@omega-services.org
 */
 
typedef struct {
	char r_name[32];
	char key[100];
	char value[1025];
	int vtype;
	int updated;
} DBEntry;

typedef struct {
	char r_name[32];
	int entry_count;
	dlink_list entries;
} DBContainer;

typedef struct {
	char FILENAME[PATH_MAX];
	char name[32]; //DB Format name IE: Sqlite3 Flatfile
	char version[100]; //DB Version 
	int uinterval;
	void *handle; //DB handler weather its an FD or SD.
	int is_open;
	char mode[4];
	time_t updated; //when DB was updated last
	dlink_list entries;
} DBInfo;

DBInfo *db;
int database_init();
int database_deinit();

int open_db_module(char *);
void add_db_function(int, DBFnct);
void del_db_function(int);
void call_db_function(int);
DBInfo *register_db(char *, char *, char *);
int  unregister_db(DBInfo *);


int add_db_entry(char *, char *, char *, ...);
int addto_db_entry(DBInfo *, char *, char *, char *, ...);
int del_db_entry(char *, char *);
 
DBEntry *get_db_entry(char *, char *);
DBEntry *getfrom_db_entry(DBInfo *, char *, char *);
DBContainer *getfrom_db_rows(DBInfo *tdb, char *rname);
DBContainer *get_db_rows(char *rname);

void print_db_entries ();

#endif  //__DBMANAGER_H__

