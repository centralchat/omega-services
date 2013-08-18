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
 *    $Id: dbmanager.c 2394 2012-06-30 15:56:22Z twitch $
 */


#include "stdinc.h"
#include "server.h"


DBContainer *create_dbcontainer(DBInfo *, char *);

static int evh_db_update()
{ 
	Event("SAVEDB", 0, NULL);
	Event("WRITEDB", 0, NULL); 
	return EVENT_OK;
}

static int evh_db_shutdown() 
{ 
	call_db_function(DBI_SAVE);
	return EVENT_OK; 
} 

static int evh_db_write()
{ 
	call_db_function(DBI_SAVE); 
	return EVENT_OK;
}

static int evh_db_null() 
{ 
	return EVENT_OK; 
}

int database_deinit() { 
	DelEvent("WRITEDB", evh_db_write);
	DelEvent("UPDATEDB", evh_db_update); 
	return 0; 
}

int database_init() {
	char *ce;
	int interval = 300;
	int i = 0;
	db = NULL;
	
	
	for (i = DBI_START; i < DBI_END; i++)
		db_handlers[i] = NULL;

	if (!(ce = (char *)get_config_entry("database", "name")))
			ce = "flatfile";
	
	if (!(open_db_module(ce)))
		return 0;
		
	printf("Loaded Database: \033[1;32m%s\033[0m\n", ce);
	
	if ((ce = (char *)get_config_entry("database", "update")))
			interval = atoi(ce);
	else
			interval = 120;

	AddEvent("UPDATEDB", evh_db_update);
	AddEventEx("Time DB Update", interval, -1, evh_db_update);
	AddEvent("WRITEDB", evh_db_write);
	return 1;
}





DBContainer *getfrom_db_rows(DBInfo *tdb, char *rname) {
	dlink_node *dl;
	DBContainer *dc, *tdc;
	
	if (!tdb) {
		alog(LOG_ERROR, "tdb was null when calling getfrom_db_rows");
		return NULL; //this should technically never happen.
	}
	
	DLINK_FOREACH(dl, tdb->entries.head) {
		dc = dl->data;
		alog(LOG_DEBUG3, "Parsing dbcontainer: %s for %s", dc->r_name, rname);
		if (strcasecmp(dc->r_name,rname) == 0) {
		
				return dc;
		}
	}
	
	return NULL;
}

DBContainer *get_db_rows(char *rname) {
	return getfrom_db_rows(db, rname);
}

DBContainer *create_dbcontainer(DBInfo *db, char *rname) { 
	DBContainer *tmp;
	dlink_node *dl;
	if (!(tmp = (DBContainer*) malloc(sizeof(DBContainer))))
			return NULL;
	memset(tmp->r_name, '\0', sizeof(tmp->r_name));
	
	strncpy(tmp->r_name, rname, sizeof(tmp->r_name));
	
	tmp->entries.head = tmp->entries.tail = NULL;
	tmp->entry_count = 0;	
	dl = dlink_create();
	dlink_add_tail(tmp, dl, &db->entries);
	
	return tmp;	
}

int del_db_entry(char *rname, char *key) {
	DBEntry *de; 
	dlink_node *dl, *tdl;
	DBContainer *dc = get_db_rows(rname);
	if (!dc)
		return 0;
	
	DLINK_FOREACH_SAFE(dl, tdl, dc->entries.head) {
			de = dl->data;
			if (strcasecmp(de->key, key)==0)
			{
					dlink_delete(dl, &dc->entries);
					dlink_free(dl);
					free(de);
					break;
			}
			
	}
	return 1;
}

DBEntry *get_db_entry(char *rname, char *key) {
	dlink_node *dl, *tdl;
	DBContainer *dc;
	DBEntry *de;
	if (!db)
		return NULL;
	DLINK_FOREACH(tdl, db->entries.head) {
		dc = tdl->data;
		if (strcasecmp(dc->r_name,rname) != 0)
				continue;
		DLINK_FOREACH(dl, dc->entries.head) {
				de = dl->data;
				
				if (strcasecmp(key, de->key)==0)
					return de;
		}
	}
	return NULL;
}

DBEntry *getfrom_db_entry(DBInfo *tdb, char *rname, char *key) {
	dlink_node *dl, *tdl;
	DBContainer *dc;
	DBEntry *de;
	if (!tdb)
		return NULL;
	DLINK_FOREACH(tdl, tdb->entries.head) {
		dc = tdl->data;
		if (!dc)
				continue;
		if (strcasecmp(dc->r_name,rname) != 0)
				continue;
		DLINK_FOREACH(dl, dc->entries.head) {
				de = dl->data;
				
				if (strcasecmp(key, de->key)==0)
					return de;
		}
	}
	return NULL;
}

int addto_db_entry(DBInfo *tdb, char *rname, char *key, char *fmt, ...) {
		
	DBEntry *de;
	DBContainer *dc = NULL;
	
	va_list args;
	dlink_node *dl;
	
	if (!tdb)
		return 0;
	
	if ((de = getfrom_db_entry(tdb, rname, key))) {
		memset(de->value, '\0', sizeof(de->value));
		va_start(args,fmt);	
		vsnprintf(de->value, sizeof(de->value), fmt, args);
		va_end(args);
		alog(LOG_DEBUG2, "Replacing db value on row %s (Key: %s)(New Val: %s)", rname, key, de->value);
		return 2; //overwritten
	}


	if (!(dc = getfrom_db_rows(tdb, rname))) {
		if (!(dc = create_dbcontainer(tdb, rname)))
				return 0;
	} 
	
	
	//if the entry was updated.
	if (!(de = (DBEntry*) malloc(sizeof(DBEntry))))
			return 0;
			
	de->vtype = 0;
	strlcpy(de->key, key, sizeof(de->key));
	strlcpy(de->r_name, rname, sizeof(de->r_name));
	
	va_start(args,fmt);	
	vsnprintf(de->value, sizeof(de->value), fmt, args);
	va_end(args);
	dl = dlink_create();
	dlink_add_tail(de, dl, &dc->entries);
	dc->entry_count++;	
	alog(LOG_DEBUG2, "Adding entry to db container (%s) %s => %s", de->r_name, de->key, de->value);		
	return 1;
}

int add_db_entry(char *rname, char *key, char *fmt, ...) {
	va_list args;
	char value[512];
	va_start(args,fmt);	
	vsnprintf(value, sizeof(value), fmt, args);
	va_end(args);	
	return addto_db_entry(db, rname, key, value);
}

void print_db_entries () {
	dlink_node	*dl, *edl;
        DBContainer	*dc;
        DBEntry		*de;

	if (!db) {
		alog (LOG_DEBUG, "print_db_entries(): Databae missing");
		return;
	}

	alog (LOG_DEBUG, "print_db_entries(): Displaying database entries");

        DLINK_FOREACH(dl, db->entries.head) {
                dc	= dl->data;

		if (!dc)
			continue;

                DLINK_FOREACH(edl, dc->entries.head) {
			de = edl->data;

			alog (LOG_DEBUG, "print_db_entries(): %s: %s => %s", dc->r_name, de->key, de->value);
                }
        }

	alog (LOG_DEBUG, "print_db_entries(): Finished displaying database entries");
}

int open_db_module(char *file) {
	int status;
	alog(LOG_MODULE, "Loading database module: %s", file); 
	status = module_open(file, MOD_TYPE_DB);
        if ((status!= 0 && status != 12))
        {
                sendto_console("FATAL: %s \033[1;34m %s \033[0m", file, GetModErr(status));
                alog(LOG_FATAL,"DB Module Error: [%s] %s",CfgSettings.protocol, GetModErr(status));
                return 0;
        }
	return 1;
}

int unregister_db(DBInfo *rdb) {
	dlink_node *dl, *tdl, *edl, *etdl;
	DBEntry *de;
	DBContainer *dc;
	
	
	if (!rdb)
		rdb = db;

	if (rdb->handle)
		call_db_function(DBI_CLOSE);
	free(rdb);
	return 1;
}

DBInfo *register_db(char *file, char *name, char *version) {
	DBInfo *tdb;
	
	if (!(tdb = malloc(sizeof(DBInfo))))
		return NULL;
	
	strlcpy(tdb->FILENAME, file, PATH_MAX);
	strlcpy(tdb->name, name, sizeof(tdb->name));
	strlcpy(tdb->version, version, sizeof(tdb->version));
	
	tdb->handle = 0;
	tdb->is_open = 0;
	tdb->updated = 0;
	tdb->uinterval = 0;
	tdb->entries.head = tdb->entries.tail = NULL;
	
	/*
	 * we don't have a MASTER DB open, allow for multiple DB formats to be called, 
	 * assume first loaded to be master DB
	 */
	db = tdb;
	return tdb;
}


void call_db_function(int DBI) {
  DBFnct fnct;
  if (!(fnct =  db_handlers[DBI]))
	return;
  fnct();
} 
void add_db_function(int DBI, DBFnct fnct)  { db_handlers[DBI] = fnct; }
void del_db_function(int DBI) { db_handlers[DBI] = NULL; }

