#include "stdinc.h"
#include "server.h"

#include "inspircd12.h"

#define NAME    "InspIRCd1.2+"
#define VERSION "$Id: inspircd12.c 2394 2012-06-30 15:56:22Z twitch $"
#define AUTHOR  "Omega Team"

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
        NULL,    // Hidden Oper, NULL because it requires m_hideoper.so, enable it if we are sent that module
        NULL
};


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, NULL);

int Module_Init () {
    inspircd12_init ();

    return MOD_CONT;
}

static int inspircd12_init () {
    int     i;

    add_protocol    (NAME, VERSION, AUTHOR, 1);

    /* Protocol Commands */
    addp_connect    (inspircd12_connect);
    addp_ping       (inspircd12_ping);
    addp_add_user   (inspircd12_nick);
    addp_join       (inspircd12_fjoin);
    addp_part       (inspircd12_part);
    addp_ping       (inspircd12_ping);
    addp_wallop     (inspircd12_wallop);
    addp_server     (inspircd12_server);
    addp_xline      (inspircd12_xline);
    addp_numeric 	(inspircd12_numeric);
    addp_kill		(inspircd12_kill);

    /* Server Commands */
    AddServCmd      ("PING",    (void *) omega_ping);
    AddServCmd      ("CAPAB",   (void *) omega_capab);
    AddServCmd      ("SERVER",  (void *) omega_server);
    AddServCmd      ("BURST",   (void *) omega_burst);
    AddServCmd      ("ENDBURST",(void *) omega_endburst);
    AddServCmd      ("UID",     (void *) omega_uid);
    AddServCmd      ("PONG",    (void *) omega_pong);
    AddServCmd      ("FJOIN",   (void *) omega_fjoin);
    AddServCmd      ("SQUIT",   (void *) omega_squit);
    AddServCmd      ("SVSNICK", (void *) omega_svsnick);
    AddServCmd      ("VERSION", (void *) omega_version);
    AddServCmd      ("ERROR",   (void *) omega_error);

    /* Ignored Server Commands */
    AddServCmd      ("ADDLINE",     (void *) omega_null);
    AddServCmd      ("DELLINE",     (void *) omega_null);
    AddServCmd      ("SNONOTICE",   (void *) omega_null);
    AddServCmd      ("METADATA",    (void *) omega_null);
    AddServCmd      ("FTOPIC",      (void *) omega_null);
    AddServCmd      ("PRIVMSG",     (void *) omega_null);
    AddServCmd      ("PUSH",        (void *) omega_null);

    /* User Commands */
    AddUserCmd      ("FMODE",   (void *) omega_fmode);
    AddUserCmd      ("PRIVMSG", (void *) omega_privmsg);
    AddUserCmd      ("PART",    (void *) omega_part);
    AddUserCmd      ("INVITE",  (void *) omega_invite);
    AddUserCmd      ("QUIT",    (void *) omega_quit);
    AddUserCmd      ("KILL",    (void *) omega_kill);
    AddUserCmd      ("ENCAP",   (void *) omega_encap);
    AddUserCmd      ("MODE",    (void *) omega_mode);
    AddUserCmd      ("OPERTYPE",(void *) omega_opertype);
    AddUserCmd      ("KICK",    (void *) omega_kick);
    AddUserCmd      ("FHOST",   (void *) omega_fhost);
    AddUserCmd      ("FNAME",   (void *) omega_fname);
    AddUserCmd      ("FIDENT",  (void *) omega_fident);
    AddUserCmd      ("NICK",    (void *) omega_nick);

    /* Ignored User Commands */
    AddUserCmd      ("OPERQUIT",(void *) omega_null);
    AddUserCmd      ("TOPIC",   (void *) omega_null);

    for (i = 0; i < 128; i++)
        ircd_umodes[i]  = umodes[i];

    return MOD_ERR_OK;
};

/** Protocol Commands **/

/* inspircd12_connect */
void inspircd12_connect () {

    /* Capabilities */
    /*
    send_line ("CAPAB START 1201");
    send_line ("CAPAB CAPABILITIES :NICKMAX=32 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=256 MAXTOPIC=308 MAXKICK=256 "
                "MAXGECOS=129 IP6NATIVE=0 IP6SUPPORT=0 MAXAWAY=201 PROTOCOL=1201 SVSPART=1 HALFOP=1");
    //send_line ("CAPAB MODULES m_hideoper.so");
    send_line ("CAPAB END");
    */

    /* Server */
    send_line ("SERVER %s %s 0 %s :%s", CfgSettings.servername, CfgSettings.pass, CfgSettings.sid, CfgSettings.desc);

    /* Burst */
    send_line (":%s BURST %d", CfgSettings.sid, time(NULL));

    /* Version */
    send_line (":%s VERSION :%s(%s)-%s#%s %s :%s", CfgSettings.sid, PACKAGE_NAME, PACKAGE_VERSION, VERSION_STRING_DOTTED, BUILD,
        CfgSettings.servername, (IRCd.ts)? "TS" : "");

    sync_state  = BURSTING;

    Event ("CONNECT", 0, NULL); // Send out the CONNECT event

    return;
}

void inspircd12_server (Link *serv) {
    send_line (":%s SERVER %s * 0 %s :%s", serv->uplink->name, serv->name, serv->sid, serv->desc);
}

/* inspircd12_ping */
void inspircd12_ping (char *server) {
    send_line (":%s PING %s %s", CfgSettings.sid, CfgSettings.sid, server);
}

/* inspircd12_nick */
void inspircd12_nick (User *user) {
    if (MyConnect (user))
        user->hideoper  = 1;

    send_line (":%s UID %s %lu %s %s %s %s %s %lu %s :%s", user->serv->sid, user->uid, (unsigned long) time(NULL),
        user->nick, user->host, user->host, user->user, "127.0.0.1", (unsigned long) time (NULL), create_usermode_string(user),
        user->realname);

    if (user->service) {
        send_line (":%s OPERTYPE Services", user->uid);

        send_line (":%s ADDLINE Q %s %s %lu 0 :Reserved for Security Services", CfgSettings.sid, user->nick,
            CfgSettings.servername, (unsigned long) time (NULL));
    }

    return;
}

/* inspircd12_fjoin */
void inspircd12_fjoin (User *u, Channel *c) {

    if (u->service) {
        send_line (":%s FJOIN %s %lu %s :o,%s", CfgSettings.sid, c->name, (unsigned long) c->channelts, create_chanmode_string (c),
            (HasId(u))? u->uid : u->nick);
    } else {
        send_line (":%s FJOIN %s %lu %s :,%s", CfgSettings.sid, c->name, (unsigned long) c->channelts, create_chanmode_string (c), (HasId(u))? u->uid : u->nick);
    }

    return;
}

void inspircd12_part (User *u, Channel *c, char *message) {
    if (!c || !u)
        return;

    send_line (":%s PART %s :%s", (HasId(u))? u->uid : u->nick, c->name, message);
    return;
}

void inspircd12_wallop (char *source, char *msg) {
    User    *u;

    if (!(u = find_uid (source)))
        send_line (":%s SNONOTICE A :%s", Omega->me->sid, msg);
    else
        send_line (":%s SNONOTICE A :%s", u->uid, msg);

    return;
}

void inspircd12_xline (char *what, char *source, char *mask, int bantime, char *reason) {
    switch (*what)
    {
        case 'G':
            send_line (":%s ADDLINE G *@%s %s %ld 0 :%s", source, mask, CfgSettings.servername, (long) time(NULL), reason);
            break;
        case 'Z':
            send_line (":%s ADDLINE Z %s %s %ld 0 :%s", source, mask, CfgSettings.servername, (long) time(NULL), reason);
            break;
    }

    return;
}

/**
 * Handle inspircd's PUSH instead of numeric tokens.
 */
 
static void inspircd12_numeric (User *target, int numeric, char *message) {
	 send_line (":%s PUSH %s ::%s %03d %s %s", Omega->me->sid, target->uid, Omega->me->name, numeric, target->uid, message);
	 return;
}

void inspircd12_kill (User *target, char *reason) {
        if (!target)
                return;
        if (!s_Guardian)
                return;

        send_line (":%s KILL %s :%s", (HasId(s_Guardian) && (IRCd.ts6 == 1))? s_Guardian->uid : s_Guardian->nick, (HasId(target) && (IRCd.ts6 == 1))? target->uid : target->nick, (reason)? reason : "No Reason");
        return;
}


/** Server Command Handlers **/

/* omega_null */
static void omega_null      (void *source, int argc, char **argv) {
    return;
}

/* omega_ping */
static void omega_ping      (Link *source, int argc, char **argv) {
    int     i;
    Link    *leaf;

    if (argc < 3)
        return;

    if ((leaf = find_serv (argv[2]))) {
        if (leaf->uplink && ((strcasecmp (leaf->uplink->sid, CfgSettings.sid)) == 0)) {
            send_line (":%s PONG %s %s", leaf->sid, leaf->sid, source->sid);
            return;
        }
    }

    send_line (":%s PONG %s %s", CfgSettings.sid, CfgSettings.sid, source->sid);
    return;
}

/* omega_capab */
/**
 * [Debug 2] RECV: CAPAB START 1202
 * [Debug 2] RECV: CAPAB MODULES m_banexception.so,m_banredirect.so,m_blockcolor.so,m_botmode.so,m_callerid.so,m_cban.so,m_chanprotect.so,m_cloaking.so,m_globops.so,m_halfop.so,m_hidechans.so,m_hideoper.so,m_noctcp.so,m_nokicks.so,m_nonicks.so,m_nonotice.so,m_ojoin.so,m_operchans.so,m_permchannels.so,m_services_account.so,m_servprotect.so,m_shun.so,m_timedbans.so
 * [Debug 2] RECV: CAPAB MODSUPPORT m_remove.so,m_sajoin.so
 * [Debug 2] RECV: CAPAB CAPABILITIES :NICKMAX=32 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=256 MAXTOPIC=308 MAXKICK=256 MAXGECOS=129 MAXAWAY=201 IP6NATIVE=1 IP6SUPPORT=1 PROTOCOL=1202 CHALLENGE=mMsDX}fa]p|wLaVqjVFZ PREFIX=(Yqaohv)!~&@%+ CHANMODES=be,k,l,CMNOPQRTcimnprst USERMODES=,,s,BHIRgikorwx SVSPART=1
 * [Debug 2] RECV: CAPAB END
 */
static void omega_capab     (Link *source, int argc, char **argv) {
    int     n, i;
    char    *params[512], modename[512], *modechar;
    char    *ce, *start;
    if ((strcasecmp (argv[1], "MODULES")) == 0) {
        if (!argv[2])
            return;

        n   = generictoken (',', 512, argv[2], params);

        for (i = 0; i < n; i++) {
            if ((strcasecmp (params[i], "m_hideoper.so")) == 0) {
                ircd_umodes[12] = "H";
            }
        }
    }
    else if ((strcasecmp (argv[1], "USERMODES")) == 0) {
        if (!argv[2])
            return;

        n   = generictoken (' ', 512, argv[2], params);

        for (i = 0; i < n; i++) {
		start = params[i];
		if ((ce = strchr(start, '='))) {
			ce++;
			strlcpy(modename, start, ce - start);
		}
	
            if ((strcasecmp (modename, "u_registered")) == 0)
                ircd_umodes[0]  = ce;
            else if ((strcasecmp (modename, "oper")) == 0)
                ircd_umodes[1]  = ce;
            else if ((strcasecmp (modename, "invisible")) == 0)
                ircd_umodes[2]  = ce;
            else if ((strcasecmp (modename, "wallops")) == 0)
                ircd_umodes[3]  = ce;
            else if ((strcasecmp (modename, "bot")) == 0)
                ircd_umodes[5]  = ce;
            else if ((strcasecmp (modename, "cloaked")) == 0)
                ircd_umodes[7]  = ce;

            //printf ("modename: %s modechar: %c\n", modename, *ce);
        }
    }
    else if ((strcasecmp (argv[1], "END")) == 0) {
    }
}

/* omega_server */
/* SERVER sleet.dwncrk.bc.ssnet.ca omega 0 225 :Waddle World */
static void omega_server    (Link *source, int argc, char **argv) {
    Link    *li, *uplink;
    int     i;

    if (source) {
        /* Source detected */
        if (!(li = new_serv (source, argv[1])))
            return;

        strlcpy (li->sid, argv[4], sizeof(li->sid));
        return;
    }

    if (!(uplink = new_serv (NULL, argv[1])))
        return;

    strlcpy (uplink->sid, argv[4], sizeof(uplink->sid));

    if (Omega->me)
        Omega->me->uplink  = uplink;
    if (!Uplink)
        Uplink      = uplink;

    Uplink->lastpong    = time(NULL);

    return;
}

/* omega_burst */
static void omega_burst     (Link *source, int argc, char **argv) {
    //sync_state  = BURSTING;
}

/* omega_endburst */
static void omega_endburst  (Link *source, int argc, char **argv) {
    if (sync_state == BURSTING) {
        Event ("BURST", 0, NULL);
        send_line (":%s ENDBURST", CfgSettings.sid);
        sync_state  = IS_CON;

    }
}

/* omega_uid */
/* :225 UID 225AAAAAA 1262411579 chaoscon 2610:1e8:2900:fffe::2 netadmin.dwncrk.bc.ssnet.ca jer 2610:1e8:2900:fffe::2 1262411584 +ios +ACGJKLOQXacdfgjkloqtx :J. Johnston */
static void omega_uid       (Link *source, int argc, char **argv) {
    User    *u;
    int     i;

    if (argc < 11) {
        printf ("Wrong parameter count for UID (%d)\n", argc);
        return;
    }

    if ((u = find_uid (argv[1])) != NULL)
        return;

    if ((u = AddUser (argv[3], argv[6], argv[4], argv[argc - 1], source->name, argv[9], argv[1], argv[5], argv[7])) == NULL) {
        printf ("Unable to add user: %s\n", argv[3]);
        return;
    }

    return;
}

/* omega_pong */
static void omega_pong  (Link *source, int argc, char **argv) {
}

/* omega_fjoin */
static void omega_fjoin (Link *source, int argc, char **argv) {
    Channel     *c;
    char        *params[1024];
    time_t      ts;
    int         i, cnt, pos;

    for (i = 0; i < 1024; i++)
        params[i]   = NULL;

    ts  = atol(argv[2]);

#if 0
    fprintf (stderr, "%d\n", argc);
    for (i = 0; i < argc; i++)
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
#endif

    if ((c = find_channel (argv[1])) != NULL) {
        if (ts < c->channelts)
            c->channelts    = ts;
    } else {
        c               = new_chan (argv[1]);
        c->channelts    = ts;
    }

    /* Sometimes we will have empty nicks for permanent channels */
    if (*argv[argc - 1] == '\0')
        return;

    /* All nicks should be at the very end */
    cnt     = generictoken (' ', 1024, argv[argc - 1], params);

    for (i = 0; i < cnt; i++) {
        while (*params[i] != ',') {
            *params[i]++;
        }

        *params[i]++;
        //fprintf (stderr, "params[%d]: %s\n", i, params[i]);

        AddToChannel (params[i], c);
    }
}

/* omega_error */
static void omega_error     (Link *source, int argc, char **argv) {
    dlink_node      *dl, *tdl, *chl, *tchl;
    Channel         *c;
    User            *u;

    close (servsock->sd);
    servsock->sd    = -1;

    exit_remote_users ();
    exit_all_servs    ();

    DLINK_FOREACH_SAFE (dl, tdl, userlist.head)
    {
        u   = dl->data;

        if (!MyConnect(u))
            continue;

        DLINK_FOREACH_SAFE (chl, tchl, u->channels.head)
        {
            c   = chl->data;

            DelFromChannel (u, c);
        }
    }

    alog (LOG_ERROR, "Failed to connect to uplink.");
    AddEventEx ("Connect Uplink", 10, 3, ev_connectuplink);

    return;
}

/* omega_fmode */
static void omega_fmode     (User *source, int argc, char **argv) {
    int     i;
    Channel *c;

    if ((c = find_channel (argv[1])) == NULL)
        return;

#if 0
    fprintf (stderr, "argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
#endif
}

/* omega_squit */
static void omega_squit     (Link *source, int argc, char **argv) {
    exit_serv (argv[1]);

    return;
}

/* omega_svsnick */
static void omega_svsnick   (Link *source, int argc, char **argv) {
    User    *u;

    if (!source || (argc < 3))
        return;

    if (!(u = find_user (argv[1])))
        return;

    strlcpy (u->nick, argv[2], sizeof(u->nick));
    return;
}

/* omega_version */
static void omega_version   (Link *source, int argc, char **argv) {
    send_line (":%s PUSH %s ::%s %03d %s %s(%s)-%s#%s %s :%s%s", CfgSettings.sid, source->sid, CfgSettings.servername, 351, source->sid,
        PACKAGE_NAME, PACKAGE_VERSION, VERSION_STRING_DOTTED, BUILD, CfgSettings.servername, (IRCd.ts)? "TS" : "",
        (IRCd.ts6)? "6" : "");
}

/* omega_privmsg */
static void omega_privmsg   (User *source, int argc, char **argv) {
}

/* omega_part */
static void omega_part      (User *source, int argc, char **argv) {
    Channel         *c;

    if (!source)
        return;

    if (!(c = find_channel (argv[1])))
        return;

    DelFromChannel (source, c);
    return;
}

/* omega_invite */
static void omega_invite    (User *source, int argc, char **argv) {
    Channel     *c;
    User        *u;

    if ((!source) || (!source->oper) || (argc < 4))
        return;

    if (!(u = find_uid (argv[1])))
        return;

    if (!(c = find_channel (argv[2])))
        return;

    AddToChannelU (u, c);

    return;
}

/* omega_quit */
static void omega_quit      (User *source, int argc, char **argv) {
    exit_user (source->nick);

    return;
}

/* omega_kill */
static void omega_kill      (User *source, int argc, char **argv) {
    User    *u;

    if (!(u = find_uid (argv[1])))
        return;

    exit_user (u->nick);
    return;
}

/* omega_encap */
static void omega_encap     (User *source, int argc, char **argv) {
    User    *u;
    Channel *c;
    int     i;

#if 0
    fprintf (stderr, "argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
#endif

    if ((strcasecmp (argv[1], CfgSettings.sid)) == 0) { // The target is us!
        if ((strcasecmp (argv[2], "SAJOIN")) == 0) {    // SAJOIN
            if (argc < 5)
                return;

            if (!(u = find_uid (argv[3])))
                return;

            if (!MyConnect (u))
                return;

            if (!(c = find_channel (argv[4]))) {
                c               = new_chan (argv[4]);
                c->channelts    = time(NULL);

                AddToChannelU (u, c);
            } else {
                AddToChannelU (u, c);
            }

            return;
        }
    }
}

/* omega_mode */
static void omega_mode      (User *source, int argc, char **argv) {
    User    *u;

    if (!(u = find_uid (argv[1])))
        return;

    read_usermode_string (u, argv[2]);
}

/* omega_opertype */
static void omega_opertype  (User *source, int argc, char **argv) {
    if (!source)
        return;

    if (!source->oper)
        read_usermode_string (source, "+o");
    return;
}

/* omega_kick */
static void omega_kick      (User *source, int argc, char **argv) {
    Channel     *c;
    User        *u;

    if (!source)
        return;

    if (!(c = find_channel (argv[1])))
        return;

    if (!(u = find_uid (argv[2])))
        return;

    KickFromChannel (u, c, argv[3]);
//    DelFromChannel (u, c);
    return;
}

/* omega_fhost */
static void omega_fhost     (User *source, int argc, char **argv) {

    if (!source || (argc < 2))
        return;

    strlcpy (source->virthost, argv[1], sizeof(source->virthost));
    return;
}

/* omega_fname */
static void omega_fname     (User *source, int argc, char **argv) {
    char    realname[50];

    if (!source || (argc < 2))
        return;

    ExpandParv (realname, 50, 1, argc, argv);

    strlcpy (source->realname, realname, sizeof(source->realname));
}

/* omega_fident */
static void omega_fident    (User *source, int argc, char **argv) {
    if (!source || (argc < 2))
        return;

    strlcpy (source->user, argv[1], sizeof(source->user));
}

/* omega_nick */
static void omega_nick (User *source, int argc, char **argv)
{
        if (!source)
                return;

        NewNick (source, argv[1]);
        source->age     = atol(argv[2]);

        return;
}

//	send_line("CAPAB CAPABILITIES :NICKMAX=32 HALFOP=1 CHANMAX=65 MAXMODES=20 IDENTMAX=12 MAXQUIT=255 PROTOCOL=1105");

/* EOF */

