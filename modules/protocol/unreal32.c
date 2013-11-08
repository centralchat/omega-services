#include "stdinc.h"
#include "server.h"

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
        "H",   // Hidden Oper
        NULL
};


/*****************************************/

#define NAME "Unreal3.2+"
#define VERSION "$Id: unreal32.c 2364 2012-02-18 13:22:32Z jer $"
#define AUTHOR  "Omega Team"
#define SUPPORTED 1

#define CLIENT_MODES "+Spqo"


/*****************************************/


void PtclFactory     ();
void unreal_connect  ();
void unreal_ping     (char *source);
void unreal_capab    (void);
void unreal_wallop   (char*,char*);
void unreal_add_user (User*);

void unreal_connect   ();
void unreal_ping      (char *source);
void unreal_capab     (void);
void unreal_wallop    (char*,char*);
void unreal_add_user  (User*);
void unreal_xline     (char *xline, char *source, char *mask, int time, char *reason);
void unreal_set_umode (User*,int,char*);
void unreal_version	 (User*);
void unreal_notice    (char*,char*,char*,...);
void unreal_set_cmode (Channel*,int,char*);
void unreal_join      (User*,Channel*);
void unreal_info      (User*);
void unreal_burst     ();
void unreal_mode      (User*,Channel*,char*);
void unreal_umode     (User*,char*);
void unreal_kill      (User *,char*);
void unreal_numeric	  (User *, int, char *);

static void omega_null		(void *, int, char **);

static void omega_quit	(User*, int, char **);
static void omega_mode      (User*, int, char **);
static void omega_privmsg   (User*, int, char **);
static void omega_ukick     (User *,int,char **);
static void omega_skick     (Link *,int,char **);

static void omega_nick	(Link*, int,  char **);
static void omega_eos       (Link*, int, char **);
static void omega_server    (Link*, int, char **);
static void omega_join      (User*, int, char **);
static void omega_sjoin     (Link*, int, char **);
static void omega_ping      (Link*, int, char **);

static void omega_part      (User *, int, char **);
static void omega_squit     (Link *, int, char **);
static void omega_svsmode   (Link *, int, char **);
static void omega_kill      (User *, int, char **);
static void omega_sajoin	(User *, int, char **);
static void omega_sapart	(User *, int, char **);
static void omega_unick     (User *, int, char **);
static void omega_svskill   (Link *, int, char **);
static void omega_skill     (Link *, int, char **);
static void omega_netinfo   (Link *, int, char **);

static void omega_error     (Link *, int, char **);
static void omega_serv_null (Link *, int, char **);
static void omega_invite	(User *, int, char **);
static void omega_rehash	(User *, int, char **);


static void omega_pong		(Link *, int, char **);
int Module_Init();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),Module_Init, NULL);

int Module_Init()
{
	int i;
   nlinks = 0;

   add_protocol(NAME,VERSION,AUTHOR,0);


   addp_connect  (unreal_connect);
   addp_ping     (unreal_ping);
   addp_wallop   (unreal_wallop);
   addp_add_user (unreal_add_user);
   addp_xline    (unreal_xline);
   addp_version  (unreal_version);
   addp_join     (unreal_join);
   addp_kill     (unreal_kill);
   addp_numeric  (unreal_numeric);

	//we actually have a part in Protocol struct for this :(
    strlcpy (ProtocolInfo.name, "unreal32", sizeof(ProtocolInfo.name));
    ProtocolInfo.ts     = 0;

	AddUserCmd ("PRIVMSG", omega_privmsg);
	AddUserCmd ("QUIT", omega_quit);
	AddUserCmd ("MODE", omega_mode);
    AddUserCmd ("PART", omega_part);
    AddUserCmd ("JOIN", omega_join);
    AddUserCmd ("KILL", omega_kill);
	AddUserCmd ("SAJOIN", omega_sajoin);
	AddUserCmd ("SAPART", omega_sapart);
	AddUserCmd ("NICK", omega_unick);
	AddUserCmd ("KICK", omega_ukick);
	AddUserCmd ("INVITE",	omega_invite);
    AddUserCmd ("REHASH", omega_rehash);


	AddServCmd ("KICK",     omega_skick);
    AddServCmd ("NETINFO",  omega_netinfo);
   	AddServCmd ("SJOIN",    omega_sjoin);
	AddServCmd ("NICK",     omega_nick);
	AddServCmd ("EOS",      omega_eos);
	AddServCmd ("SERVER",   omega_server);
    AddServCmd ("PING",     omega_ping);
    AddServCmd ("SQUIT",    omega_squit);
    AddServCmd ("SVSMODE",  omega_svsmode);
    AddServCmd ("SVS2MODE", omega_svsmode);
    AddServCmd ("SVSKILL",  omega_svskill);
    AddServCmd ("KILL",     omega_skill);
    AddServCmd ("PONG",		omega_pong);

    AddServCmd ("SWHOIS",	omega_serv_null);
    AddServCmd ("SENDSNO",	omega_serv_null);
    //AddServCmd ("ERROR",        omega_error);

    for (i = 0; i < 128; i++)
        ircd_umodes[i] = umodes[i];

	return MOD_ERR_OK;
}



/*****************************************/

void unreal_capab(void)
{
	send_line("PROTOCTL NICKv2 NICKIP SJOIN2 SJOIN VHP SJ3 NOQUIT TKLEXT VL");
}

/*****************************************/

void unreal_server(void)
{
    send_line("SERVER %s %d :U0-*-%s %s", CfgSettings.servername, 1, CfgSettings.numeric,CfgSettings.desc);
    
    Event ("CONNECT", 0, NULL);
    
    send_line("NETINFO 0 %lu 0 * 0 0 0 :%s", time(NULL), CfgSettings.network);
    send_line("EOS");
}


/*****************************************/

void unreal_connect()
{
     nlinks = 1;
     sync_state = BURSTING; //set our state to bursting.

     unreal_capab ();
     send_line("PASS :%s", CfgSettings.pass);

     unreal_server();

}
/*****************************************/
void unreal_numeric(User *target, int numeric, char *text) {
	send_line (":%s %d %s %s",Omega->me->name, numeric, target->nick, text);
}

/*****************************************/

void unreal_ping(char *source)
{
    send_line("%s %s :PONG",CfgSettings.servername,source);
}

/*****************************************/

void unreal_wallop(char *source,char *msg)
{
	send_cmd(source,"GLOBOPS",msg);

}

/*****************************************/
void unreal_add_user(User* user)
{
	Channel* c;

    if (MyConnect (user))
        user->hideoper  = 1;
	
    //NICK Scanner 2 1212765286 security ip78.216-86-156.static.steadfast.net failboat.zomgirc.net 1 +iowgxWqtp staff.zomgirc.net 2FacTg== :ZOMGIRC Proxy Scanning Service
    send_line("NICK %s 1 0 %s %s %s 0 %s %s * :%s", user->nick,user->user,user->host,CfgSettings.servername, create_usermode_string(user), user->host, user->realname);

    if (user->service)
        unreal_xline("Q", CfgSettings.servername, user->nick, 0, "Reserved For Services");
    return;
}

/*****************************************/



void unreal_notice(char *source,char *dest,char *msg, ...)
{
	char	*tmp;
	va_list	args;

	if (!dest || !msg)
		return;

	if (!source)
		source = CfgSettings.servername;


	tmp = (char*) malloc(sizeof(char) * 512);
	va_start(args,msg);
	vsnprintf(tmp,512,msg,args);

	send_line(":%s NOTICE %s :%s",source,dest,tmp);

	va_end(args);
	free(tmp);
	tmp = NULL;

	return;

}


/*****************************************/

void unreal_join (User *u, Channel *c)
{
    if (!c || !u)
        return;

    if (sync_state == BURSTING)
        send_line ("SJOIN %lu %s %s :%s", (unsigned long) c->channelts, c->name,
                create_chanmode_string (c), u->nick);
    else {
        if (u->service) {
            send_line (":%s SJOIN %lu %s %s :%s", CfgSettings.servername, (unsigned long) c->channelts,
                c->name, create_chanmode_string (c), u->nick);
        } else {
            send_line (":%s SJOIN %lu %s %s :%s", CfgSettings.servername, (unsigned long) c->channelts,
                            c->name, create_chanmode_string (c), u->nick);
        }
    }

	return;

}

void unreal_part (User *u, char *c, char *message)
{
	if (!c || !u)
		return;

	send_line (":%s PART %s :%s", u->nick, c, message);
	return;
}

/*****************************************/


void unreal_xline(char *xline, char *source, char *mask, int bantime, char *reason)
{
	//rickroll.zomgirc.net TKL + Q * PENIS services.zomgirc.net 0 1212365461 :Reserved for services
    send_line(":%s TKL + %s * %s %s 0 %lu :%s",CfgSettings.servername,xline,mask,source,(unsigned long) time(NULL),reason);
	return;

}

/*****************************************/

void unreal_version (User* user)
{
	send_line (":%s 351 %s %s %s-%s#%s :%s %s", CfgSettings.servername, user->nick, PACKAGE_NAME, PACKAGE_VERSION, VERSION_STRING_DOTTED, BUILD, CfgSettings.servername,VERSION);
}

void unreal_kill (User *source, char *reason)
{
	send_line (":%s KILL %s :%s", CfgSettings.servername, source->nick, reason);
}

/*****************************************/

static void omega_null	(void *source, int argc, char **argv)
{
	return;
}

//NICK Scanner 2 1217111688 security ip78.216-86-156.static.steadfast.net failboat.zomgirc.net 1 +iowgxWqtp staff.zomgirc.net 2FacTg== :ZOMGIRC Proxy Scanning Service

static void omega_nick  (Link* li, int argc, char **argv)
{
	User*	user;
	struct in_addr addr;
    uint32 ipaddr;

    if ((argc > 3) && !(argc < 9))
    {
        if ((user = find_user (argv[1])) != NULL)
                return;

        ipaddr = ntohl(decode_ip(argv[10]));
        addr.s_addr = htonl(ipaddr);

        user = AddUser (argv[1], argv[4], argv[5], argv[11],
                        argv[6], argv[8],NULL, argv[9], str_ntoa(addr));

        return;
    }
}

/*****************************************/
static void omega_netinfo(Link * li, int argc, char **argv)
{
    return;    
}

/*****************************************/


static void omega_server (Link* lk, int argc, char **argv)
{
    Link    *li;

        //we use NL + VS we shouldnt have less then 2 params
        //and ARGV[1] should always hold NAME :/


	if ((argc <= 2) || (!argv[1]))
	{
		alog(3,"Invalid Params Received in SERVER BURST");
		return;
	}

    if (argv[0])
    {
        if (!(li = new_serv (lk, argv[1])))
            return;

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


/*****************************************/

static void omega_eos (Link *lk, int argc, char **argv)
{
    Link    *li, *local;

    if (!(li = find_serv (argv[0])))
        return;

    if (!(local = find_serv (CfgSettings.servername)))
        return;

    if (!(local->uplink))
        return;

    if ((strcasecmp (local->uplink->name, argv[0])) == 0)
    {
        if (sync_state == BURSTING)
        {
            Event ("BURST", 0, NULL);
            sync_state  = IS_CON;

            //XXX - Fix sync bug was trying to send wallops through li->name which wasn't our server
            ircd_wallop ((s_Guardian)? s_Guardian->nick : CfgSettings.servername, "\002LINK:\002 \002%s\002 has finished "
                        "syncing with \002%d\002 servers", CfgSettings.servername, link_count());
            return;
        }
    }

    return;

}


/*****************************************/

/**
 ** [Debug 2] RECV: :failboat.zomgirc.net SJOIN 1214691301 #Sanctuary :wadachicken
 ** [Debug 2] RECV: SJOIN 1214690751 #Omega +ntr :@~Mitch Jeremy @~Omega @~Rob @~chaoscon @+~ph33r @Mitch[Screen] @~Jer Amanda +CIA-1 Amanda-Work
 */

static void omega_sjoin (Link *source, int argc, char **argv)
{
    int     i, cnt;

    char    *params[512];
    Channel *c;
	User	*u;

	if (argc < 4)
		return;

    if (!(c = find_channel (argv[2])))
        c = new_chan (argv[2]);

    if (!(c))
        return;

    for (i = 0; i < 512; i++)
        params[i]  = NULL;

    if (c->channelts > atol(argv[1]))
        c->channelts    = atol(argv[1]);

    if (argc > 4)   // SJOIN contain modes
    {
        if (argc > 5)
            params[0]   = argv[4];
        if (argc > 6)
            params[1]   = argv[5];
        if (argc > 7)
            params[2]   = argv[6];
        if (argc > 8)
            params[3]   = argv[7];

        read_chanmode_string (c, argv[3], params);
    }

    cnt = generictoken (' ', 512, argv[argc - 1], params);

    for (i = 0; i < cnt; i++)
        AddToChannel (params[i], c);

	return;
}

static void omega_join  (User *source, int argc, char **argv)
{
    Channel     *c;
    dlink_node  *dl, *tdl;

    if (!source)
        return;

    if ((strcasecmp (argv[1], "0")) == 0)
    {
        printf ("Removing %s from all channels\n", source->nick);

        DLINK_FOREACH_SAFE (dl, tdl, source->channels.head)
        {
            c   = dl->data;

            DelFromChannel (source, c);
        }
    }

    return;
}

/*****************************************/

static void omega_mode  (User *source, int argc, char **argv)
{
    char    *params[15];
    int     i;
    Channel *c;

    if (!source)
        return;

    if (*argv[1] == '#')
    {
        for (i = 0; i < 15; i++)
            params[i] = NULL;

        if ((c = find_channel (argv[1])))
        {
            if (argc > 3)
                i   = generictoken (' ', 15, argv[3], params);

            read_chanmode_string (c, argv[2], params);
            return;
        }
        return;
    }

    read_usermode_string (source, argv[2]);

    return;
}

/*****************************************/

static void omega_privmsg(User* u, int argc, char **argv)
{
	return;
}

/*****************************************/

static void omega_quit	(User *u, int argc, char **argv)
{
	exit_one_user (u, NULL);
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
}

static void omega_part  (User *user, int argc, char **argv)
{
    Channel     *c;

    if (!user)
        return;

    if (argc < 2)
        return;

    if (!(c = find_channel (argv[1])))
        return;

    DelFromChannel (user, c);
    return;
}

static void omega_squit (Link *source, int argc, char **argv)
{
    exit_serv (argv[1]);
    return;
}

static void omega_svsmode   (Link *source, int argc, char **argv)
{
    User    *u;

    if (!source)
        return;

    if (!(u = find_user (argv[1])))
        return;

    read_usermode_string (u, argv[2]);
    return;
}

static void omega_kill (User *src, int argc, char **argv)
{

	User* u;
	Channel* c;
	dlink_node *dl,*tdl;

	if ((u = isOurClient(argv[1])))
	{
 		unreal_add_user(u);

		//rejoin our channels
		 DLINK_FOREACH_SAFE(dl,tdl, u->channels.head)
       		 {
            		c   = dl->data;
			unreal_join (u,c);

		 }
		return;
	}
	else
		exit_user(argv[1]);

	return;

}


static void omega_svskill (Link *src, int argc, char **argv)
{

	User* u;
	Channel* c;
	dlink_node *dl,*tdl;

    if (!last_attempt)
        last_attempt = time(NULL) + 600;

    //reset counters
    if ((time(NULL) > last_attempt) && (introduce_attempts < 3))
    {
            introduce_attempts = 0;
            last_attempt = time(NULL) + 600;
    }

    if ((!introduce_attempts) || (introduce_attempts <= 3))
        introduce_attempts++;
    else {
        alog(3,"Unable to introduce users tried %d times.",introduce_attempts);
        exit(0);
    }


	if ((u = isOurClient(argv[1])))
	{
 		unreal_add_user(u);

		//rejoin our channels
		 DLINK_FOREACH_SAFE(dl,tdl, u->channels.head)
       	 {
            c = dl->data;
			unreal_join (u,c);

		 }
		return;
	}
	else
		exit_user(argv[1]);

	return;

}

static void omega_skill (Link *src, int argc, char **argv)
{

	User* u;
	Channel* c;
	dlink_node *dl,*tdl;

    if (!last_attempt)
        last_attempt = time(NULL) + 600;

    //reset counters
    if ((time(NULL) > last_attempt) && (introduce_attempts < 3))
    {
            introduce_attempts = 0;
            last_attempt = time(NULL) + 600;
    }

    if ((!introduce_attempts) || (introduce_attempts <= 3))
        introduce_attempts++;
    else {
        alog(3,"Unable to introduce users tried %d times.",introduce_attempts);
        exit(0);
    }


	if ((u = isOurClient(argv[1])))
	{

	    //issue a kill for whoever the hell collided with us
	     unreal_kill (find_user(argv[1]), "Nick collision - your nickname was overruled");
 	     unreal_add_user(u);

		//rejoin our channels
		 DLINK_FOREACH_SAFE(dl,tdl, u->channels.head)
       	 	{
          		  c = dl->data;

			      unreal_join (u,c);

		 }
		 return;
	}
	else
		exit_user(argv[1]);

	return;

}


/*
void unreal_kill(char* src,char* trg,char *msg)
{
        send_line(":%s KILL %s :%s (%s)",(src)? src : CfgSettings.servername,trg,CfgSettings.servername,msg);
        return;
}
*/



static void omega_sajoin (User *src, int argc, char **argv)
{
	Channel		*c;
	User		*user;

	if (argc < 3)
		return;

	if (!src)
		return;

	if (!(user = find_user (argv[1])))
		return;

	if (!(c = find_channel(argv[2])))
		return;

	AddToChannelU (user, c);
	unreal_join (user, c);

	return;
}

static void omega_sapart (User *source, int argc, char **argv)
{
	Channel		*c;
	User		*user;
	char		message[255];

	if (argc < 3)
		return;

	if (!source)
		return;

	if (!(user = find_user (argv[1])))
		return;

	if (!(c = find_channel (argv[2])))
		return;

	snprintf (message, 255, "SAPART from %s", source->nick);

	DelFromChannel (user, c);
	unreal_part (user, argv[2], message);

	return;
}

static void omega_unick(User* u, int ac, char **av)
{
	NewNick(u,av[1]);
	return;
}

static void omega_pong	(Link *source, int argc, char **argv)
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

//[Debug 2] RECV: :Mitch KICK #Omega G2 :Mitch
static void omega_ukick(User *u, int argc, char **argv)
{

	Channel *c;
       User *trg;

	if (!(c = find_channel(argv[1])))
		return;

	if (!(trg = find_user(argv[2])))
		return;

	DelFromChannel (trg, c);
	return;
}

static void omega_skick(Link *li, int argc, char **argv)
{
	Channel *c;
       User *trg;

	if (!(c = find_channel(argv[1])))
		return;

	if (!(trg = find_user(argv[2])))
		return;

	DelFromChannel (trg, c);
	return;
}

static void omega_error (Link *src, int argc, char **argv)
{
	alog(3,"Received ERROR from the server - exiting");
	Exit(0);
	return;

}

static void omega_invite (User *source, int argc, char **argv)
{
	Channel	*c;
	User	*u;

	if (!source)
		return;

	if (argc < 3)
		return;

	if (source->oper != 1)
	{
		sendto_one_notice (s_Guardian, source, "Permission Denied");
		return;
	}

	if (!(c = find_channel (argv[2])))
		return;

	if (!(u = find_user (argv[1])))
		return;

	AddToChannelU (u, c);
	unreal_join (u, c);
	return;
}

static void omega_rehash (User *source, int argc, char **argv)
{
    if (!source)
        return;

    ircd_wallop (s_Guardian->nick, "%s is rehashing the server", source->nick);
    Rehash (0);
    return;
}

static void omega_serv_null (Link* li, int argc, char **argv)
{
	return;
}




