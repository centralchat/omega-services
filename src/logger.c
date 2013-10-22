#include "setup.h"
#include "stdinclude.h"
#include "appinclude.h"


const char * log_type_str[] = {
	"INFO",
	"WARNING",
	"NOTICE",
	"ERROR",
	"CRITICAL",
	"FATAL",
	"DEBUG",
	"DEBUG2",
	"DEBUG3",
	"MODULE",
	"CONFIG",
	"DATABASE",
  "SOCKET",
  "EVENT"
};


static FILE * logfd;
static unsigned long curday = 0;


/*********************************************************/
/** 
 * If we dont have a log file open a new one
 * else we need to rotate logs and determine 
 * or do nothing that works too.
 */

int log_open() 
{
  // char    timestamp[32];

  // time_t      t;
  // struct tm   *tmp = NULL;

  // memset(&t, 0, sizeof(time_t));

  // time (&t);
  // tmp = localtime (&t);
  // if (logfd)
  // {
  //     if (tmp->tm_mday == curday)
  //       return 1;
  //     else
  //       log_close();
  // }

  memset(log_name, '\0', sizeof(log_name));

  // snprintf (log_name, sizeof(log_name), "%s/%d%d%d.log",
  //   LOG_DIR, (tmp->tm_year + 1900), (tmp->tm_mon + 1), tmp->tm_mday);
  snprintf (log_name, sizeof(log_name), "%s/%s.log",
    LOG_DIR, "security");

  // fprintf (stderr, "Opening log file: %s\n", log_name);
  // curday  = tmp->tm_mday;
  if (!logfd)
    logfd   = fopen (log_name, "a");
  
  if (!logfd)
  {
    fprintf (stderr, "Warning: Could not open logfile (%s): %s\n", log_name, strerror(errno));
  	return 0;
  }
  return 1;
}

/**********************************************************/

void log_close (void)
{

  if (logfd)
  {
    fflush(logfd);
    fclose(logfd);
  }
  return;
}

/**********************************************************/


void __attribute__((format(printf, 5, 0))) log_message
(int level, const char *file, int line, const char *function, const char *fmt, ...)
{
  //TODO: Make this memory dynamic alloc
  char * message = NULL;

  va_list ap;

  switch (level) 
  {
    case _LOG_DEBUG:
      if (core.debug < 1)
        return;
    case _LOG_DEBUG2:
      if (core.debug < 2)
        return;
    case _LOG_DEBUG3:
      if (core.debug < 3)
        return;
    default:
      break;
  }

  if (!log_open())
    return;

  va_start(ap, fmt);
  if (!(vasprintf(&message, fmt, ap)))
    return;

  //Console logging
  if (core.nofork)
    fprintf(stderr, "[%s] %s %s(%d): %s\n", log_type_str[level], file,
      function, line, message);    

  fprintf(logfd, "[%s] %s %s(%d): %s\n", log_type_str[level], file,
      function, line, message);  

  free(message);
  fflush(logfd);
  va_end(ap);

}



