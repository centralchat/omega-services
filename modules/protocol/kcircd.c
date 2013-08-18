#include "stdinc.h"
#include "server.h"
#include "kcircd.h"

#define NAME        "Ratbox 2.2+"
#define VERSION     "$Revision: 1071 $"
#define AUTHOR      "Omega Team"
#define SUPPORTED   1

static char	sid[20];

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

int nlinks;

extern User    *user_list;

int Module_Init();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, NULL);

int Module_Init()
{
    int i;

    nlinks  = 1;

	sid[0] = '\0';


    add_protocol (NAME, VERSION, AUTHOR, 1);

    addp_connect    (kcircd_connect);
    addp_ping       (kcircd_ping);
    addp_wallop     (kcircd_wallop);
    addp_add_user   (kcircd_nick);
    addp_join       (kcircd_join);
	addp_numeric	(kcircd_numeric);
	
    strlcpy (ProtocolInfo.name, "kcircd", sizeof(ProtocolInfo.name));
    ProtocolInfo.ts		= 1;

    AddServCmd  ("NICK",    (void *) omega_nick);
    AddServCmd  ("UID",     (void *) omega_uid);
    AddServCmd  ("PING",    (void *) omega_ping);
    AddServCmd  ("SJOIN",   (void *) omega_sjoin);
    AddServCmd  ("PASS",    (void *) omega_pass);
    AddServCmd  ("SQUIT",   (void *) omega_squit);
    AddServCmd  ("SERVER",  (void *) omega_server);
    AddServCmd  ("SID",     (void *) omega_sid);
    AddServCmd  ("ENCAP",   (void *) omega_encap);
    AddServCmd  ("MODE",    (void *) omega_mode);
	AddServCmd	("KILL",	(void *) omega_skill);

	AddServCmd	("SVINFO",	(void *) omega_null);
	AddServCmd	("NOTICE",	(void *) omega_null);

    AddUserCmd  ("PRIVMSG", (void *) omega_privmsg);
    AddUserCmd  ("QUIT",    (void *) omega_quit);
    AddUserCmd  ("MODE",    (void *) omega_mode_user);
    AddUserCmd  ("TMODE",   (void *) omega_tmode);
	AddUserCmd	("PART",	(void *) omega_part);
	AddUserCmd  ("JOIN",	(void *) omega_join);
	AddUserCmd	("KILL",	(void *) omega_kill);
	AddUserCmd	("NICK",	(void *) omega_unick);

    for (i = 0; i < 128; i++)
        ircd_umodes[i]  = umodes[i];

	return MOD_ERR_OK;
}

void kcircd_numeric(User *target, int numeric, char *text) {
	send_line (":%s %d %s %s", (IRCd.ts6) ? Omega->me->sid : Omega->me->name, numeric, HasId(target) ? target->uid : target->nick, text);
}


static void kcircd_mode (Link *server, User *user, Channel *channel, char *modes, char *modeparams)
{
	char	*params[15];
	int		i;

	for (i = 0; i < 15; i++)
		params[i] = NULL;

	i = generictoken (' ', 15, modeparams, params);

    if (channel)
    {
		read_chanmode_string (channel, modes, params);
        return;
    }
    else
    {
        read_usermode_string (user, modes);
        return;
    }

#if 0
    if ((user) && !(server))
    {
        if (*modes == '#')
            return;

        read_usermode_string (u, modes);
        return;
    }

    else if (server)
    {
        if (*modes == '#')
            return;

        if (!user)
            return;

        read_usermode_string (u, modes);

        return;
    }
#endif

    return;
}

void kcircd_connect()
{
    nlinks++;

	if (IRCd.ts6)
	{
		send_line ("PASS %s TS 6 :%s", CfgSettings.pass, CfgSettings.sid);
		send_line ("CAPAB :QS EX CHW IE KLN GLN KNOCK UNKLN CLUSTER ENCAP SAVE SAVETS_100");
		send_line ("SERVER %s 1 %s", CfgSettings.servername, CfgSettings.desc);
		send_line ("SVINFO 6 3 0 :%lu", (unsigned long) time(NULL));
	}
	else
	{
        send_line ("PASS %s :TS", CfgSettings.pass);
        send_line ("CAPAB :QS EX CHW IE KLN GLN KNOCK UNKLN CLUSTER ENCAP SAVE SAVETS_100");
        send_line ("SERVER %s 1 :%s", CfgSettings.servername, CfgSettings.desc);
        send_line ("SVINFO 5 3 0 :%lu", (unsigned long) time(NULL));
	}

    sync_state  = BURSTING;

	Event("CONNECT", 0, NULL);

    return;
}


void kcircd_wallop(char *source, char *msg)
{
    User    *u;

    u = find_user (source);

	/* Servernames in a WALLOP are ugly on ratbox, if we find one default to Guardian */
	if ((strcasecmp(source, CfgSettings.servername)) == 0)
		send_line (":%s WALLOPS :%s", (IRCd.ts6)? s_Guardian->uid : s_Guardian->nick, msg);
	else
		send_line (":%s WALLOPS :%s", (IRCd.ts6)? u->uid : u->nick, msg);
	return;
}

void kcircd_ping (char *source)
{
	if (sync_state == BURSTING)
	{
        Event ("BURST", 0, NULL);
        sync_state  = IS_CON;
//		ircd_wallop ("Guardian", "End of Burst");
	}

	send_line (":%s PONG :%s", (IRCd.ts6)? CfgSettings.sid : CfgSettings.servername, source);
}

void kcircd_nick (User* user)
{

	if (IRCd.ts6)
	{
		send_line (":%s UID %s 1 %lu %s %s %s %s %s :%s", CfgSettings.sid, user->nick,
					(unsigned long) time(NULL), create_usermode_string(user), user->user, user->host, "127.0.0.1", user->uid, user->realname);
	}
	else
	{
		send_line ("NICK %s 1 %lu %s %s %s %s :%s", user->nick, (unsigned long) time(NULL),
				"+oa", user->user, user->host, CfgSettings.servername, user->realname);
	}

    if (user->registered)
        send_line (":%s ENCAP * SU %s :%s", CfgSettings.servername, user->nick, user->nick);

    send_line (":%s RESV * %s :Reserved for Security Services", s_Guardian->nick, user->nick);

}

void kcircd_join (User *u, Channel *c)
{
    if (!u || !c)
        return;

    send_line (":%s SJOIN %ld %s %s :@%s", (IRCd.ts6)? CfgSettings.sid : CfgSettings.servername, (unsigned long) c->channelts, c->name, create_chanmode_string (c), (HasId(u) && (IRCd.ts6 == 1))? u->uid : u->nick);

//    AddToChannelU (u, c);
    return;
}

/*
** Command Handlers
**
*/

static void omega_null	(Link *source, int argc, char **argv)
{
	return;
}

/**
 ** [0] - Source
 ** [1] - Nickname
 ** [2] - Hops
 ** [3] - TS
 ** [4] - Modes
 ** [5] - Ident
 ** [6] - Host
 ** [7] - Server
 ** [8] - Realname
 */
static void omega_nick	(Link *li, int argc, char **argv)
{
	User	*user;

	if (argc > 3)
	{
		if ((user = find_user (argv[1])) != NULL)
			return;

		if ((user = AddUser (argv[1], argv[5], argv[6], argv[8], argv[7], argv[4], NULL, NULL, NULL)) == NULL)
			return;

		return;
	}

    else
    {
        if (ProtocolInfo.ts6 == 1)
            user    = find_uid (argv[0]);
        else
            user    = find_user (argv[0]);

        if (user)
        {
            NewNick (user, argv[1]);
            user->age   = atol(argv[2]);

            return;
        }

        printf ("Unable to find an entry for: %s\n", argv[0]);

        return;
    }

    return;
}

static void omega_unick	(User *source, int argc, char **argv)
{
	if (!source)
		return;

	NewNick (source, argv[1]);
	source->age	= atol(argv[2]);

	return;
}

/**
 ** [0] - Source
 ** [1] - Nickname
 ** [2] - Hops
 ** [3] - TS
 ** [4] - Modes
 ** [5] - Ident
 ** [6] - Hostname
 ** [7] - IP
 ** [8] - UID
 ** [9] - Realname
 */

//User* AddUser(char *nick, char *user, char *host, char *realname, char *server, char *modes, char *uid, char *vhost, char *ip)

static void omega_uid	(Link *li, int argc, char **argv)
{
	User	*user;

	if (argc < 9) {
		printf ("Wrong parameter count for UID (%d)\n", argc);
		return;
	}

	if ((user = find_uid (argv[8])) != NULL)
		return;

	if ((user = AddUser (argv[1], argv[5], argv[6], argv[9], argv[0], argv[4], argv[8], NULL, strdup(argv[7]))) == NULL) {
        printf ("Unable to add user %s\n", argv[1]);
		return;
    }

	return;
}

static void omega_quit	(User *u, int argc, char **argv)
{
	exit_user (u->nick);

	return;
}

//:stormy.dwncrk.bc.ssnet.ca SJOIN 1213491852 #Flood +tn :SmartServ

/**
 ** [0] - Source
 ** [1] - TS
 ** [2] - Channel
 ** [3] - Modes
 ** [4] - Users
 */

static void omega_sjoin     (Link *source, int argc, char **argv)
{
    Channel     	*c;
	char			*params[1024];
	time_t			ts;

    int         i, cnt, pos;

	for (i = 0; i < 1024; i++)
		params[i] = NULL;

	ts = atol(argv[1]);

    if ((c = find_channel (argv[2])) != NULL)
    {
		if (ts < c->channelts)
			c->channelts	= ts;
        // Check TS
    }
    else
    {
        c   = new_chan (argv[2]);
        c->channelts    = ts;
    }

	/**
	 ** Wow do I ever hate SJOIN now, our key and/or limit are sent AFTER the mode string
     ** No idea exactly how the fuck we parse it, could just pass it in read_chanmode_string
     ** and do the dirty work there. Dunno, lack of sleep could be catching up and Im not
	 ** thinking straight.
	 **
	 ** I can only imagine what it looks like on Unreal, I am SO not looking forward to that
	 **
	 ** - Jer
	 */

	if (argc > 4) /* Fucking modes anyway, if we are above 4, we could have a key AND/OR limit */
		params[0]	= argv[4];

	if (argc > 5) // FUCK OFF SJOIN, Seriously this is a cluster fuck
		params[1]	= argv[5];

	read_chanmode_string (c, argv[3], params);

	/**
     ** Now that we are done dealing with the ugliness of SJOIN its time to move on to
	 ** adding users to the channel
	 */

	pos	= 4;

	if (argc > 5)
		pos	= 5;
	if (argc > 6)
		pos	= 6;


	cnt	= generictoken (' ', 1024, argv[pos], params);

	for (i = 0; i < cnt; i++)
		AddToChannel (params[i], c);

	return; // Return for now, I really have to sort this bullshit out
}

static void omega_privmsg   (User *u, int argc, char **argv)
{
    return;
}

static void omega_server    (Link *source, int argc, char **argv)
{
    Link    *li, *uplink;

    if (argc < 4)
        return;

    if (argv[0])
    {
        /**
         ** We have a source, a remote server is introducing a server
         ** First we search to make sure the sending server has been introduced
         */

		if (!(li = new_serv (source, argv[1])))
			return;

        return;
    }
    else
    {
        /**
         ** We don't have a source, assume this is our uplink server
         ** introducing itself
         */

        if (!(uplink = new_serv (NULL, argv[1])))
            return;

		if (sid[0] != '\0')
			strlcpy (uplink->sid, sid, sizeof(uplink->sid));

		if (Omega->me)
			Omega->me->uplink	= uplink;

		Uplink	= uplink;

        return;
    }


    return;
}

/**
 ** [Debug 2] RECV: :42X SID ircd.botbay.net 2 5A1 :Botbay's Primary Server
 **
 ** [0] - Source
 */

static void omega_sid       (Link *source, int argc, char **argv)
{
    Link    *li, *uplink;

	if (!(li = new_serv (source, argv[1])))
	{
		printf ("Uplink not found (%s)\n", argv[1]);
		return;
	}

	strlcpy (li->sid, argv[3], sizeof(li->sid));

    return;
}

static void omega_mode      (Link *source, int argc, char **argv)
{
    User    *u;
    Channel *c;

    if (argc < 3)
        return;

    if (*argv[1] == '#')
        return;

    if ((u = find_user(argv[1])))
    {
        return;
    }

#if 0

    u = find_user (argv[0]);

    if (!u)
        find_uid (argv[0]);

    if (u)
        read_usermode_string (u, argv[2]);
#endif

    return;
}

static void omega_ping  (Link *source, int argc, char **argv)
{
    ircd_ping (argv[1]);
}

static void omega_pass  (Link *source, int argc, char **argv)
{
    Link    *li;

    if (argc < 5)
	{
		if (IRCd.ts6 > 0)
			IRCd.ts6 = 0;
        return;
	}

	if (sid[0] == '\0')
		strlcpy (sid, argv[4], sizeof(sid));

    return;
}

static void omega_squit (Link *source, int argc, char **argv)
{
    exit_serv (argv[1]);
    return;
}

static void omega_encap (Link *source, int argc, char **argv)
{
    User    *u;

    if (argc < 2)
        return;

    if (strcasecmp (argv[2], "GCAP") == 0)
    {
        return;
    }
    else if (strcasecmp (argv[2], "SU") == 0)
    {
        if ((u = find_user (argv[3])))
            u->registered   = 1;
        return;
    }

    return;
}

static void omega_mode_user (User *source, int argc, char **argv)
{
    if (argc < 3)
        return;

    if (*argv[1] == '#')
        return;

    kcircd_mode (NULL, source, NULL, argv[2], NULL);
    return;
}

static void omega_tmode     (User *source, int argc, char **argv)
{
    Channel     *c;
    char        params[15];

    if (argc < 3)
        return;

	params[0] = '\0';

	if ((c = find_channel (argv[2])))
	{
		if (argc > 3)
			ExpandParv (params, 15, 4, argc, argv);

    	kcircd_mode (NULL, source, c, argv[3], params);
		return;
	}

    return;
}

/**
 ** [Debug 2] RECV: :J PART #Omega [NON TS6]
 **
 */

static void omega_part	(User *source, int argc, char **argv)
{
	Channel			*c;
	struct ChanUser	*cu;
	dlink_node		*dl;

	if (!source)
		return;

	if (!(c = find_channel (argv[1])))
		return;

	DelFromChannel (source, c);
	return;
}

/**
 ** Interesting fact, this is only ever hit when TS6 is enabled
 ** TS5 servers use SJOIN all the time
 */
static void omega_join	(User *source, int argc, char **argv)
{
	Channel			*c;
    dlink_node      *dl, *tdl;

	if (!source)
		return;

    if ((strcasecmp (argv[2], "0")) == 0)
    {
        DLINK_FOREACH_SAFE (dl, tdl, source->channels.head)
        {
            c   = dl->data;

            DelFromChannel (source, c);
        }

        return;
    }

	if (!(c = find_channel (argv[2])))
		return;

	AddToChannelU (source, c);
	return;
}

static void omega_kill	(User *source, int argc, char **argv)
{
	User	*user;

	if (!source)
		return;

	if (!(user = find_user (argv[1])))
		return;

	exit_user (user->nick);
}

static void omega_skill	(Link *source, int argc, char **argv)
{
	User	*user;

	if (!source)
		return;

	if (!(user = find_user(argv[1])))
		return;

	exit_user (user->nick);
}

