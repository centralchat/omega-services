#include "stdinclude.h"
#include "appinclude.h"

int eos;
int nlinks;

static char *umodes[128] = {
  "r",   // Registered Nickname
  "o",   // IRC Operator
  "i",   // Invisible
  "w",   // Wallops
  "z",   // Using a secure connection
  "B",   // Is a bot
  "S",   // Is a network service
  "x",   // Has a cloaked host
  "v",   // Has a virtual host
  "p",   // Private
  "A",   // Server Administrator
  "V",   // Web TV
  "H",    // Hidden Oper
  NULL
};


/*****************************************/

#define NAME "Unreal3.2+"
#define VERSION "$Id: unreal32.c 2364 2012-02-18 13:22:32Z jer $"
#define AUTHOR  "Omega Team"
#define SUPPORTED 1

#define CLIENT_MODES "+Spqo"


/********************************************************************/

int module_init();
void module_deint();

void unreal32_server(void);

event_return_t unreal32_uplink(args_t *);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,9,0),module_init, module_deint);

int module_init()
{
  event_add("UPLINK", unreal32_uplink);
	return MOD_ERR_OK;
}

void module_deint()
{

}

/********************************************************************/
/*                        EVENT HANDLEING                           */
/********************************************************************/

event_return_t unreal32_uplink(args_t * args)
{
  unreal32_server();
  return EVENT_SUCCESS;
}


/********************************************************************/
/*                          COMMANDS                                */
/********************************************************************/


void unreal32_server()
{
    socket_write(core.socket, "SERVER %s %d :U0-*-%s %s", 
      core.settings.server.name, 1, core.settings.server.numeric, "Omega Security");

    event_dispatch("CONNECT", NULL);
}



