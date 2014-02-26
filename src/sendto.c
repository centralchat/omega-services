#include "stdinclude.h"
#include "appinclude.h"




/********************************************************************/
/*  
 * Send a IRC formated line
 */

int sendto_uplink(char * fmt, ...)
{
  char  tmp[1024];

  int n;
  va_list args;

  if (!fmt)
    return -1;

  memset(tmp,0,sizeof(tmp));

  va_start(args,fmt);
  vsnprintf(tmp,1024,fmt,args);
  va_end (args);

  if (!strstr(tmp,"\r")) { strcat(tmp,"\r"); }
  if (!strstr(tmp,"\n")) { strcat(tmp,"\n"); }

  if (strlen(tmp) > 512)
    return -1;

  return socket_write(core.socket, tmp);
}