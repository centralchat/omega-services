

#include "stdinc.h"
#include "server.h"


#define NAME "InspIRCd1.1+"
#define VERSION "$Id: inspircd.c 2200 2012-01-08 06:50:35Z jer $"
#define AUTHOR  "Omega Team"

#define SUPPORTED 1



static char *umodes[128] = {
        "r",   // Registered Nickname
        "o",   // IRC Operator
        "i",   // Invisible
        "n",   // Wallops
        "z",   // Using a secure connection
        NULL,   // Is a bot
        NULL,   // Is a network service
        "x",   // Has a cloaked host
        "v",   // Has a virtual host
        "p",   // Private
        NULL,  // Server Administrator
	 NULL,	 // Web TV
        NULL
};


void insp_capab();

static void insp_connect ();
static void insp_ping    ();


static void omega_pong	(Link *source, int argc, char **argv);
static void omega_ping      (Link* li, int argc, char **argv);
static void omega_fjoin	(Link* li, int argc, char **argv);
static void omega_fmode     (Link* li, int argc, char **argv);
static void omega_server    (Link* li, int argc, char **argv);

int nlinks;
/*****************************************/


int Module_Init();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, NULL);

int Module_Init()
{

	throwModErr("%s","This protocol is not supported at the current time.");
	int i;

	addp_connect  (insp_connect);
	addp_ping     (insp_ping);

	AddServCmd("FJOIN",omega_fjoin);

       for (i = 0; i < 128; i++)
       	 	ircd_umodes[i] = umodes[i];


	return MOD_CONT;

}


/*****************************************/


static void insp_connect()
{
	insp_capab();
	send_line("SERVER %s %s 0 :%s",CfgSettings.servername,CfgSettings.pass,CfgSettings.desc);
	return;
}




/*****************************************/

void insp_capab()
{

	send_line("CAPAB START");
	send_line("CAPAB CAPABILITIES :NICKMAX=32 IPV6NATIVE=0 IPV6SUPPORT=0 HALFOP=1 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=255 PROTOCOL=1105");
	send_line("CAPAB END");
}

/*****************************************/

static void insp_ping(char *source)
{
    send_line(":%s PONG %s %s",CfgSettings.servername,source,CfgSettings.servername);
    return;
}

/*****************************************/


static void omega_ping (Link* li, int argc, char **argv)
{
	if (!li)
	{
		li = find_serv(CfgSettings.servername);
        	ircd_ping(li->uplink->name);
       	return;
	}
	ircd_ping (li->name);
	return;
}

/******************************************/


static void omega_pong(Link *source, int argc, char **argv)
{
	if (!source || !Uplink)
		return;

	if ((strcasecmp (source->name, Uplink->name)) == 0)
	{
		Uplink->lastpong	= time(NULL);
		return;
	}

	return;
}

/*******************************************/
/**
 * following line pulled from InspIRCd s<>s
 * documentation.
 *
 * :test.chatspike.net FJOIN #test 1133992411 :,Omster ,Brain ,DesktopOm ,Cyan ,w00teh ,Ghost ,Brain2
 */


static void omega_fjoin (Link* li, int argc, char **argv)
{

	int     i, cnt;
	Channel *c;


	if (argc > 5)
		return;

   	if (!(c = find_channel (argv[1])))
		c = new_chan (argv[1]);

	if (!(c))
		return;

       if (c->channelts > atol(argv[2]))
       	c->channelts  = atol(argv[2]);

	for (i = 4; i < argc; i++)
		AddToChannel (argv[i], c);

	return;

}


/*******************************************/

//:<source server or nickname> FMODE <target> <timestamp> <modes and parameters>
//:LI FMODE #opers 115432135 +bb *!user@spy.com *!person@watching.us

static void omega_fmode (Link* li, int argc, char **argv)
{
	char *params[512];

	Channel* c;
	if (!(c = find_channel(argv[1])))
		return;

	read_chanmode_string (c, argv[3], params);

	return;
}


/*******************************************/



static void omega_server (Link* lk, int argc, char **argv)
{

    Link    *li, *uplink;


    if ((argc <= 2) || (!argv[1]))
		alog(3,"Invalid Params Received in SERVER BURST");

    if (argv[0])
    {
		if (!(li = new_serv (lk, argv[1])))
			return;
    }
    else
    {
        if (!(li = new_serv (NULL, argv[1])))
            return;


		Uplink	= li;
		Uplink->lastpong	= time(NULL);

		if (Omega->me)
			Omega->me->uplink	= li;
        return;
    }

}




/* EOF */

