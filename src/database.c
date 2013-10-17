#include <stdinclude.h>
#include <appinclude.h>



// MYSQL *db_connection = NULL;

void * db_query(char *, ...);

/**************************************************************/

int db_connection_init() 
{
  // db_connection = mysql_init(NULL);
  // if (!db_connection) 
  // {
  //   fprintf(stderr, "%s\n", mysql_error(db_connection));
  //   return 0;
  // }
  return 1;
}

/**************************************************************/

int db_connect(char * host, char * user, char * password) 
{ 

  // if (!db_connection) {
  //   if (!db_connection_init()) return 0;
  // }

  // if (mysql_real_connect(db_connection, host, user, password, NULL, 0, NULL, 0) == NULL) 
  // {
  //   fprintf(stderr, "%s\n", mysql_error(db_connection));
  //   mysql_close(db_connection);
  //   return 0;
  // }  
  return 1;
}

/**************************************************************/

void * db_query(char * query_fmt, ...)
{
  // //TODO Make this resizing
  // char query_str[8129];
  // va_list ap;
  // va_start(ap, query_fmt);
  // vsnprintf(query_str, 8129, query_fmt, ap);
  // va_end(ap);
  return NULL; //mysql_query(db_connection, query_str);
}



void db_close() 
{
  // mysql_close(db_connection);
}