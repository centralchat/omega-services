#include "stdinclude.h"
#include "appinclude.h"

/************************************************************/
/*                      Variables                           */
/************************************************************/


Socket * cli_sock;


/************************************************************/
/*                      Prototypes                          */
/************************************************************/

int cli_create_socket(char * path);


/************************************************************/
/*                  Module (un)Loading                      */
/************************************************************/

/************************************************************/

int cli_load()
{
  return MOD_ERR_OK;
}

/************************************************************/

void cli_unload()
{

}

/************************************************************/
/*                   Internal Functions                     */
/************************************************************/


/************************************************************/

int cli_create_socket(char * path)
{
    cli_sock = socket_new();
    cli_sock->flags |= SOCK_UNIX;

    log_message(LOG_INFO, "Creating cli socket on %s", path);
  
    int ret = se_unix_connect(core.socket, path);
    if (!socket_addto_list(core.socket))
      return FALSE;

    return ret;
}

/************************************************************/
/*                  Handlers/Callbacks                      */
/************************************************************/

/************************************************************/


void cli_accept_callback(Socket * s)
{
  socket_write(s, "This is a test\n");
}



/************************************************************/


MODREGISTER("CLI", 
  "Omega Development",
  "0.0.1", 
  MAKE_ABI(0,9,0), 
  cli_load, 
  cli_unload
);

