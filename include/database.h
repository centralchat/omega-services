#ifndef __DATABASE_H__
#define __DATABASE_H__

typedef struct _database_info { 
	char host[MAXHOST];
	char user[MAXUSER];
	char pass[MAXPASS];
	char driver[32];
	char file[MAXPATH];
} db_info_t;

typedef struct database { 
	db_info_t * dbi;
	void * connection;
	int in_use;
} Database;

int db_init  (dlink_list * databases);
int db_connection_init  ();
int db_connect          (char *, char *, char *);

// MYSQL_RES * db_query(char *, ...);


#endif 