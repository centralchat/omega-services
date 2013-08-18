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
 *    $Id: flatfile.c 2387 2012-04-16 19:01:24Z jer $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME "flatfile"
#define VERSION "$Id: flatfile.c 2387 2012-04-16 19:01:24Z jer $"
#define AUTHOR "Twitch"


static int  db_load();
static void db_close();
void db_parse(char *);
//DB Implements
//static void dbi_open();
static int dbi_save();
static int dbi_load();

#define DB_FILE EPATH "/security.db"
#define DB_VERSION "2"
#define DB_NAME "flatfile"

#define DB_READ_MODE "r"
#define DB_WRITE_MODE "w+"

char *current_file;


DBInfo *flatdb;

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),db_load, db_close);

static char dbfile[255];

static int db_load()
{
	DBEntry *de;
	
	snprintf(dbfile, 254, "%s/security.db",datadir);
	
	if (!(flatdb = register_db(dbfile, DB_NAME, DB_VERSION)))
			return MOD_STOP;
			
	add_db_function(DBI_SAVE, dbi_save);
	add_db_function(DBI_LOAD, dbi_load);
	if (db_file_exists (flatdb->FILENAME)) {
			addto_db_entry(flatdb, "DB", "I", "%s %d %lu", DB_NAME, DB_VERSION, time(NULL));	
			dbi_load();
	}
	return MOD_CONT;
}

static void db_close()
{
	//dbi_save(); //force DB save up delete.
	unregister_db(flatdb);
	return;
}

static int dbi_load() {
	load_flat_db(flatdb->FILENAME);
	return 1;
}

static int dbi_save() {
	dlink_node *dl, *sdl, *stdl, *tdl;
	DBEntry *de;
	DBContainer *dc;
	FILE *fd;
	
	if (db_file_exists (dbfile))
		unlink(dbfile);
		
	if (!(fd = fopen(dbfile, DB_WRITE_MODE))) {
			alog(LOG_ERROR, "Cannot open DB file for reading: %s", dbfile);
			return 0;
	}
	DLINK_FOREACH_SAFE(dl, sdl, flatdb->entries.head) {
			dc = dl->data;
			alog(LOG_DEBUG2, "Container: %s", dc->r_name);
			DLINK_FOREACH_SAFE(tdl, stdl, dc->entries.head) {
				de = tdl->data;
				fprintf(fd, "%s %s %s\n", de->r_name, de->key, de->value);
				alog(LOG_DEBUG2, "Saving db entry: (%s) %s => %s", de->r_name, de->key, de->value);
			}
	}
	fflush(fd);
	fclose(fd);
	return 1;
}

int load_flat_db(char *file)
{
	FILE *fp;
		
        int db_fd;
        struct stat stat_buf;
        int ret = 0;
        int t = 0;
        char *db_buffer= NULL;

        if (!(db_fd = open(file, O_RDONLY)))
        {
			fp = fopen(file, "w+");
			fprintf(fp, "DB I %s %s %lu\n", DB_NAME, DB_VERSION, time(NULL));
			fclose(fp);
        }
		
    	if (!(db_fd = open(file, O_RDONLY))) {
    		    alog(LOG_ERROR, "Couldn't fstat \"%s\": %s\n", file, strerror(errno));
            	    return 0;
    	}
		
        if (fstat(db_fd, &stat_buf) == -1)
        {
                alog(LOG_ERROR, "Couldn't fstat \"%s\": %s\n", file, strerror(errno));
                close(db_fd);
                return 0;
        }
        if (!stat_buf.st_size)
        {
                close(db_fd);
                return 0;
        }
        if (!(db_buffer = (char *)malloc(stat_buf.st_size + 2)))
        {
                alog(LOG_ERROR, "Unable to create a buffer for config file.\n");
                close(db_fd);
                return 0;
        }
        ret = read(db_fd, db_buffer, stat_buf.st_size);
        if (ret != stat_buf.st_size)
        {
                alog(LOG_ERROR, "Error while reading file: %s\n", file);
                free(db_buffer);
                close(db_fd);
        }
        close(db_fd);
		db_buffer[ret] = '\0';
        current_file = file;
        db_parse(db_buffer);
        
        if (db_buffer)
           free(db_buffer);
           
        db_buffer = NULL; 
        
        return 1;
}


void db_parse(char *buffer) {
	char *pos = NULL, *line = NULL, *p = NULL;
	char *params[3];
	int cnt = 0;

	pos = strdup(buffer);
	while (1)
	{		
			if ((line = strchr(pos, '\n'))==NULL) 
						break;
			*line++ = '\0';
			alog(LOG_DEBUG3, "db_prase: parsing line (%s)\n", pos);
			for (cnt = 0; cnt <= 3; cnt++)
				params[cnt] = NULL;

			cnt = 0;
			while ((p = strchr(pos, ' ')) != NULL) { 
					params[cnt++] = pos;
					if (cnt==3)
						break;
					*p++ = '\0';
					pos = p;
			}
			if (cnt < 3)
				break;
			alog(LOG_DEBUG3, "Adding DB entry %s::%s => %s\n", params[0], params[1], params[2]);	
			addto_db_entry(flatdb, params[0], params[1], params[2]);
			pos = line;
	}
    // memset(pos, '0', strlen(buffer));
    // MyFree(pos);
	return;
}


int db_file_exists (char * fileName)
{
   FILE *fp;   
   if (fp = fopen(fileName,"r"))
   {	
		fclose(fp);
		return 1;
   }
   return 0;
}



