#include "stdinc.h"
#include "server.h"

#include "inspircd20.h"

#define NAME    "InspIRCd2.0"
#define VERSION "$Id: inspircd12.c 1908 2010-05-28 21:40:35Z jer $"
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


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Init, NULL);

int Module_Init () {
    inspircd_init ();
    return MOD_CONT;
}

static int inspircd_init () {
    int     i;

    add_protocol    (NAME, VERSION, AUTHOR, 1);

    /* Protocol Commands */
    addp_connect    (inspircd_connect);
    addp_ping       (inspircd_ping);
    addp_add_user   (inspircd_nick);
    addp_join       (inspircd_fjoin);
    addp_part       (inspircd_part);
    addp_wallop     (inspircd_wallop);
    addp_server     (inspircd_server);
    addp_numeric  (inspircd_numeric);
    addp_mode	   (inspircd_mode);
    addp_kill      (inspircd_kill);
   
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
    AddUserCmd      ("KICK",	 (void *) omega_kick);
    AddUserCmd      ("FHOST",     (void *) omega_fhost);
    AddUserCmd      ("FNAME",    (void *) omega_fname);
    AddUserCmd      ("FIDENT",   (void *) omega_fident);
    AddUserCmd      ("NICK",     (void *) omega_nick);

    /* Ignored User Commands */
    AddUserCmd      ("OPERQUIT",(void *) omega_null);
    AddUserCmd      ("TOPIC",   (void *) omega_null);
	
    for (i = 0; i < 128; i++)
        ircd_umodes[i]  = umodes[i];

    return MOD_ERR_OK;
};

/** Protocol Commands **/

/* inspircd_connect */
void inspircd_connect () {

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

void inspircd_mode	(User *u,Channel *c,char *mode) {
       char *params[15], m[100]; 
       int i, cnt;
       i   = generictoken (' ', 15, mode, params);
       if (!u || !c)
       		return;
       		
       snprintf(m,sizeof(m)-1,"%s", params[0]);
         
       for (cnt = 1; cnt < i; cnt++) 
       		params[cnt-1] = params[cnt];
 
 	alog(LOG_DEBUG, "inspircd_mode: Setting [%s]", m);
 	for (i = 0; params[i]; i++)
 		alog(LOG_DEBUG3, "inspircd_mode: %s", params[i]);
	send_line(":%s FMODE %s %lu %s", (u->uid)? u->uid : u->nick, c->name, c->channelts, mode);
}
void inspircd_server (Link *serv) {
    send_line (":%s SERVER %s * 0 %s :%s", serv->uplink->name, serv->name, serv->sid, serv->desc);
}

/* inspircd_ping */
void inspircd_ping (char *server) {
    send_line (":%s PING %s %s", CfgSettings.sid, CfgSettings.sid, server);
}

/* inspircd_nick */
void inspircd_nick (User *user) {
    if (MyConnect (user))
        user->hideoper  = 1;
    if (!user->serv || !user->uid)
        return;

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

/* inspircd_fjoin */
void inspircd_fjoin (User *u, Channel *c) {

    if (u->service) {
        send_line (":%s FJOIN %s %lu %s :o,%s", CfgSettings.sid, c->name, (unsigned long) c->channelts, create_chanmode_string (c),
            (HasId(u))? u->uid : u->nick);
    } else {
        send_line (":%s FJOIN %s %lu %s :,%s", CfgSettings.sid, c->name, (unsigned long) c->channelts, create_chanmode_string (c), (HasId(u))? u->uid : u->nick);
    }

    return;
}

void inspircd_part (User *u, Channel *c, char *message) {
    if (!c || !u)
        return;

    send_line (":%s PART %s :%s", (HasId(u))? u->uid : u->nick, c->name, message);
    return;
}

void inspircd_wallop (char *source, char *msg) {
    User    *u;

    if (!(u = find_uid (source)))
        send_line (":%s SNONOTICE A :%s", Omega->me->sid, msg);
    else
        send_line (":%s SNONOTICE A :%s", u->uid, msg);

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

/**
 * Handle inspircd's PUSH instead of numeric tokens.
 */
static void inspircd_numeric (User *target, int numeric, char *message) {
	 send_line (":%s PUSH %s ::%s %03d %s %s", Omega->me->sid, target->uid, Omega->me->name, numeric, target->uid, message);
	 return;
}

void inspircd_kill (User *target, char *reason) {
	if (!target)
		return;
	if (!s_Guardian)
		return;

	send_line (":%s KILL %s :%s", (HasId(s_Guardian) && (IRCd.ts6 == 1))? s_Guardian->uid : s_Guardian->nick, (HasId(target) && (IRCd.ts6 == 1))? target->uid : target->nick, (reason)? reason : "No Reason");
	return;
}

/* omega_capab */

static void omega_capab     (Link *source, int argc, char **argv) {
    int     n, i, pos, cnt;
    char    *params[512], modename[512];
    char    *ce, *start, tmp[2];

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
	pos  = 127; //unused mode position.
        for (i = 0; i < n; i++) {
		ce = params[i]; 
		cnt = 0;
		tmp[0] = '\0';
		while (*ce != '=')
		{
				modename[cnt] = *ce;
				cnt++; ce++;
		}
		ce++;
	   	tmp[0] = *ce;
             if ((strcasecmp (modename, "u_registered")) == 0)
                	pos = 0;
            else if ((strcasecmp (modename, "oper")) == 0)
                	pos = 1;
            else if ((strcasecmp (modename, "invisible")) == 0)
              		pos = 2;
            else if ((strcasecmp (modename, "wallops")) == 0)
                	pos = 3;
            else if ((strcasecmp (modename, "bot")) == 0)
              		pos = 4;
            else if ((strcasecmp (modename, "cloaked")) == 0)
               		pos = 5;
                ircd_umodes[pos] = tmp;
                alog(LOG_DEBUG, "Adding compat for: (%s) %s", modename, ircd_umodes[pos]);
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
    int     i, cnt;
    Channel *c;
    char *params[15];

    for (i = 0; i <= 15; i++)
    		params[i] = NULL;
    		
    if ((c = find_channel (argv[1])) == NULL)
        return;

    if (argc < 4)
    	return;
    	
   //apply modes
   cnt = 0;
   if (argc > 3) {
	   for (i = 4; i < argc; i++)  {
	   	params[cnt] = argv[i];
	   }	  
   } 
    read_chanmode_string (c, argv[3], params);
    alog(LOG_DEBUG2, "Setting modes [%s] to %s", argv[3], c->name);
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
    if (!source) {
	alog (LOG_DEBUG2, "No source provided for OPERTYPE.");
        return;
    }

    if (!source->oper) {
	alog (LOG_DEBUG2, "Applying oper to %s.", source->nick);
        read_usermode_string (source, "+o");
    }
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

