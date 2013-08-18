#include "stdinc.h"
#include "server.h"
#include "bahamut.h"

#define NAME	"Bahamut 1.8+"
#define VERSION	"$Id: bahamut.c 2200 2012-01-08 06:50:35Z jer $"
#define AUTHOR	"Jer"
#define SUPPORTED	1

static char *umodes[128] = {
        NULL,    // Registered Nickname
        "o",    // IRC Operator
        "i",    // Invisible
        "w",    // Wallops
        NULL,   // Using a secure connection
        NULL,   // Is a bot
        NULL,   // Is a network service
        NULL,   // Has a cloaked host
        NULL,   // Has a virtual host
        NULL,   // Private
        "a",    // Server Administrator
        NULL,    // Web TV
        NULL
};

int Module_Init();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, NULL);


int Module_Init ()
{
	int i;


	add_protocol (NAME, VERSION, AUTHOR, 1);

	addp_connect	(bahamut_connect);
	addp_add_user	(bahamut_nick);
	addp_join		(bahamut_join);
	addp_ping		(bahamut_ping);

	AddServCmd ("SERVER",	(void *) omega_server);
	AddServCmd ("PING",		(void *) omega_ping);
	AddServCmd ("SVINFO",	(void *) omega_null);
	AddServCmd ("NICK",		(void *) omega_nick);
	AddServCmd ("PASS",		(void *) omega_null);
	AddServCmd ("GNOTICE",	(void *) omega_null);
	AddServCmd ("CAPAB",	(void *) omega_null);
	AddServCmd ("SQLINE",	(void *) omega_null);
	AddServCmd ("SJOIN",	(void *) omega_sjoin);
	AddServCmd ("LUSERSLOCK",	(void *) omega_null);

	AddUserCmd ("NICK",		(void *) omega_nick);

    for (i = 0; i < 128; i++)
        ircd_umodes[i]  = umodes[i];

	return MOD_ERR_OK;
}

void bahamut_connect ()
{
	send_line ("PASS %s :TS", CfgSettings.pass);
	send_line ("CAPAB :BURST NICKIP TSMODE SSJOIN NOQUIT UNCONNECT");
	send_line ("SERVER %s 1 :%s", CfgSettings.servername, CfgSettings.desc);
	send_line ("SVINFO 5 3 0 :%lu", (unsigned long) time(NULL));

	sync_state	= BURSTING;

	Event ("CONNECT", 0, NULL);

	return;
}

void bahamut_nick (User *user)
{
	send_line ("NICK %s 1 %lu %s %s %s %s 0 %s :%s", user->nick, (unsigned long) time(NULL), create_usermode_string (user),
				user->user, user->host, user->serv->name, "127.0.0.1", user->realname);

	send_line (":%s SQLINE %s :Reserved for Security Services", CfgSettings.servername, user->nick);

	return;
}

void bahamut_join (User *user, Channel *chan)
{
	send_line (":%s SJOIN %lu %s %s :@%s", CfgSettings.servername, (unsigned long) chan->channelts, chan->name,
				create_chanmode_string (chan), user->nick);

	return;
}

void bahamut_ping (char *source)
{
	if (sync_state == BURSTING)
	{
		Event ("BURST", 0, NULL);
		sync_state	= IS_CON;
	}

	send_line (":%s PONG :%s", CfgSettings.servername, source);
	return;
}


/**
 **
 */

static void omega_null		(void *source, int argc, char **argv)
{
	return;
}

static void omega_server	(Link *source, int argc, char **argv)
{
	Link	*li;

	if (argc < 4)
		return;

	if (argv[0])
	{
		if (!(li = new_serv (source, argv[1])))
			return;

		return;
	}
	else
	{
		if (!(li = new_serv (NULL, argv[1])))
			return;

		if (Omega->me)
			Omega->me->uplink	= li;

		Uplink	= li;

		return;
	}

	return;
}

static void omega_ping		(Link *source, int argc, char **argv)
{
	ircd_ping (argv[1]);
	return;
}

/* [Debug 2] RECV: NICK Jer 1 1218172706 +oi jer typhoon.dwncrk.bc.ssnet.ca squall.dwncrk.bc.ssnet.ca 0 3232235561 :J. Johnston */

static void omega_nick		(void *source, int argc, char **argv)
{
	User	*user;

	if (argc > 3)
	{
		if ((user = find_user (argv[1])) != NULL)
			return;

		if ((user = AddUser (argv[1], argv[5], argv[6], argv[10], argv[7], argv[4], NULL, NULL, NULL)) ==
NULL)
			return;
	}
	else
	{
		user	= find_user (argv[0]);

		if (user)
		{
			NewNick (user, argv[1]);

			return;
		}
	}
}

static void omega_sjoin		(Link *source, int argc, char **argv)
{
	Channel		*chan;
	char		*params[1024];
	time_t		ts;
	int			i, cnt, pos;

	for (i = 0; i < 1024; i++)
		params[i] = NULL;

	ts	= atol(argv[1]);

	if ((chan = find_channel (argv[2])) != NULL)
	{
		if (ts < chan->channelts)
			chan->channelts		= ts;
	}
	else
	{
		chan			= new_chan (argv[2]);
		chan->channelts	= ts;
	}

	return;
}

