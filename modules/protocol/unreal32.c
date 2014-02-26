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

int  module_init();
void module_deint();

void unreal32_negotiate(void);

event_return_t unreal32_evt_update(args_t *);
int            unreal32_cmd_server(args_t *);
int            unreal32_cmd_ping  (args_t *);



MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,9,0),module_init, module_deint);

int module_init()
{

  event_add("UPLINK", unreal32_evt_update);

  command_server_add("SERVER", unreal32_cmd_server);
  command_server_add("PING"  , unreal32_cmd_ping);

	return MOD_ERR_OK;
}

void module_deint()
{

}


/********************************************************************/
/*                         Server Commands                          */
/********************************************************************/
/* Handle inbound SERVER command
 * 
 * [DEBUG2] parser.c parser_handle_line(110): RECV: SERVER irc.omega-services.test 1 :U2311-DFhinXOoE-1 FooNet Server
 * [DEBUG3] parser.c parser_handle_line(155): para[1]: irc.omega-services.test
 * [DEBUG3] parser.c parser_handle_line(155): para[2]: 1
 * [DEBUG3] parser.c parser_handle_line(155): para[3]: U2311-DFhinXOoE-1 FooNet Server
 */


int unreal32_cmd_server(args_t * args)
{
  server_t * s    = NULL;
  args_t   * arg  = NULL;

  if (!(s = server_init(args->argv[1], " ", args->argv[2])))
    return FALSE;

  ARGS(arg, s, 0, NULL);
  event_dispatch("SERVER", arg);

  return TRUE;
}



int unreal32_cmd_ping(args_t * args)
{
  sendto_uplink("%s %s :PONG",core.settings.server.name, args->source);
}




/********************************************************************/
/*                        EVENT HANDLEING                           */
/********************************************************************/

event_return_t unreal32_evt_update(args_t * args)
{
  unreal32_negotiate();
  return EVENT_SUCCESS;
}


/********************************************************************/
/*                       Internal Functions                         */
/********************************************************************/


void unreal32_negotiate()
{
  sendto_uplink("PROTOCTL NICKv2 NICKIP SJOIN2 SJOIN VHP SJ3 NOQUIT TKLEXT VL");
  sendto_uplink("PASS :%s", core.settings.link.pass);
  sendto_uplink("SERVER %s %d :U0-*-%s Omega",core.settings.server.name, 1, core.settings.server.numeric);
  event_dispatch("CONNECT", NULL);
}




