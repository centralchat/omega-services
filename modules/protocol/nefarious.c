#include "stdinc.h"
#include "server.h"

#define NAME    "Nefarious IRCu 1.2+"
#define VERSION "$Id$"
#define AUTHOR  "Omega Team"
static char id[3];

static int  nefarious_init      ();
static int  nefarious_deinit    ();

void nefarious_connect      ();
void nefarious_ping         (char *);
void nefarious_nick         (User *);
void nefarious_join         (User *, Channel *);
void nefarious_numeric		(User *, int, char *);
static void omega_server    (Link *, int, char **);
static void omega_ping      (Link *, int, char **);
static void omega_eb        (Link *, int, char **);
static void omega_nick      (Link *, int, char **);
static void omega_squit     (Link *, int, char **);

static void omega_b         (Link *, int, char **);

static void omega_join      (User *, int, char **);
static void omega_quit      (User *, int, char **);
static void omega_privmsg   (User *, int, char **);

MODHEADER (NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5), nefarious_init, NULL);

static int nefarious_init () {
    add_protocol (NAME, VERSION, AUTHOR, 1);

    IRCd.p10    = 1;

    addp_connect    (nefarious_connect);
    addp_ping       (nefarious_ping);
    addp_add_user   (nefarious_nick);
    addp_join       (nefarious_join);
	addp_numeric	(nefarious_numeric);

    AddServCmd      ("SERVER",  omega_server);
    AddServCmd      ("S",       omega_server);
    AddServCmd      ("G",       omega_ping);    // PING
    AddServCmd      ("EB",      omega_eb);
    AddServCmd      ("N",       omega_nick);
    AddServCmd      ("SQ",      omega_squit);

    AddServCmd      ("B",       omega_b);   // Channel burst?

    AddUserCmd      ("J",       omega_join);
    AddUserCmd      ("C",       omega_join);
    AddUserCmd      ("Q",       omega_quit);
    AddUserCmd      ("P",       omega_privmsg);

    return MOD_ERR_OK;
}


void nefarious_numeric(User *target, int numeric, char *text) {
	send_line (":%s %d %s %s", (IRCd.ts6) ? Omega->me->sid : Omega->me->name, numeric, HasId(target) ? target->uid : target->nick, text);
}

static int nefarious_deinit () {
}
/*
char * generate_uid (void)
{
        int     i;

        for (i = 8; i > 3; i--)
        {
                if (nefarious_uid[i] == 'Z')
                {
                        nefarious_uid[i] = '0';
                        return nefarious_uid;
                }
                else if (nefarious_uid[i] != '9')
                {
                        nefarious_uid[i]++;
                        return nefarious_uid;
                }
                else
                        nefarious_uid[i] = 'A';
        }

        if (nefarious_uid[3] == 'Z')
        {
                nefarious_uid[i] = 'A';
                s_assert (0);
        }
        else
                nefarious_uid[i]++;

        return nefarious_uid;
}
*/

/*
static void nefarious_uidgen (User *u) {
    int     i;

    if (nefarious_uid[0] == '\0')    // Init the uid list
    {
        alog (LOG_DEBUG2, "Initializing nefarious_uid");
        for (i = 0; i < 2; i++)
            nefarious_uid[i]  = id[i];
        for (i = 2; i < 5; i++)
            nefarious_uid[i]  = 'A';

        nefarious_uid[6] = '\0';

        strlcpy (u->uid, nefarious_uid, 6);
        return;
    }

    alog (LOG_DEBUG2, "nefarious_uid: %s", nefarious_uid);

    for (i = 4; i > 2; i--)
    {
        if (nefarious_uid[i] == 'Z')
        {
            nefarious_uid[i] = '0';
            strlcpy (u->uid, nefarious_uid, 6);
            return;
        }
        else if (nefarious_uid[i] != '9')
        {
            nefarious_uid[i]++;
            strlcpy (u->uid, nefarious_uid, 6);
            return;
        }
        else
            nefarious_uid[i] = 'A';
    }

    if (nefarious_uid[2] == 'Z')
    {
        nefarious_uid[i] = 'A';
        s_assert (0);
    }
    else
        nefarious_uid[i]++;

    strlcpy (u->uid, nefarious_uid, 6);
    return;
}*/

/** Outgoing Protocol Commands **/
void nefarious_connect ()
{
    inttobase64 (id, atoi(CfgSettings.numeric), 2); // This should be safe, numeric should be denied in rehashes

    send_line ("PASS :%s", CfgSettings.pass);
    send_line ("SERVER %s 1 %li %li J10 %s]]] +s6 :%s", CfgSettings.servername, (unsigned int)time(NULL),
        (unsigned int)time(NULL), id, CfgSettings.desc);

    sync_state  = BURSTING;

    Event ("CONNECT", 0, NULL);
    return;
}

void nefarious_ping (char *source)
{
    send_line ("%s G :%s", id, source);
}

void nefarious_join (User *u, Channel *c)
{
    if (sync_state == BURSTING) {
        send_line ("%s B %s %lu + %s:o", id, c->name, c->channelts, u->uid);
    }
    else {
        send_line ("%s J %s %lu", u->uid, c->name, c->channelts);
    }
}

/* [Debug 2] RECV: Az N O3 2 1262913224 O3 X3.AfterNET.Services +oik AAAAAA AzAAA :Oper Service Bot */
/* usermode is excluding if the server is not bursting */
void nefarious_nick (User *u)
{
    if (!u)
        return;

/*
    if ((!HasId (u)) || (strlen (u->uid) > 5)) {
        alog (LOG_DEBUG2, "Generating UID for %s", u->nick);
        nefarious_uidgen (u);
    }
*/
    
    if (sync_state == BURSTING)
        send_line ("%s N %s 1 %lu %s %s %s %s %s :%s", id, u->nick, (unsigned long) time (NULL), u->user, u->host, create_usermode_string (u), "AAAAAA", u->uid, u->realname);
    else {
        send_line ("%s N %s 1 %lu %s %s %s %s :%s", id, u->nick, (unsigned long) time (NULL), u->user, u->host, "AAAAAA", u->uid, u->realname);
        send_line ("%s M %s %s", u->uid, u->nick, create_usermode_string (u));
    }

    return;
}

static void omega_server    (Link *source, int argc, char **argv)
{
    char    id[3];
    Link    *li, *uplink;
    int i;

    if (argc == 9) {    // Uplink
        strlcpy (id, argv[6], 3);

        if (!(uplink = new_serv (NULL, argv[1])))
            return;

        strlcpy (uplink->sid, argv[6], 3);

        if (Omega->me)
            Omega->me->uplink  = uplink;
        if (!Uplink) {
            Uplink              = uplink;
            Uplink->lastpong    = time(NULL);
        }

        return;
    }

    if (!(li = new_serv (source, argv[2])))
        return;

    strlcpy (li->sid, argv[7], 3);
    return;
}

static void omega_ping      (Link *source, int argc, char **argv)
{
    uint32  ts, tsnow, value;

    tsnow   = time(NULL);

    if (*argv[2] == '!') {
        *argv[2]++;

        ts      = strtol(argv[2], NULL, 10);
        value   = tsnow - ts;

        send_line ("%s Z %s %ld %ld %ld %ld", id, source->name, (long int) ts, (long int) tsnow, (long int) value, (long int) time(NULL));
    } else
        send_line ("%s Z %s %ld 0 %ld %ld", id, source->name, (long int) tsnow, (long int) tsnow, (long int) time(NULL));

    return;
}

static void omega_eb        (Link *source, int argc, char **argv)
{
    if (sync_state == BURSTING) {
        Event ("BURST", 0, NULL);
    }

    send_line ("%s EB", id);
    send_line ("%s EA", id);

    sync_state = IS_CON;
}

/* Why does IRCu make it so fucked up?! */

/* [Debug 2] RECV: AB N chaoscon_ 1 1262919782 jer 192.168.0.250 +i DAqAD6 ABAAD :J. Johnston  - during burst (argv: 11) */
/**
argc: 11
argv[0]: AB
argv[1]: N
argv[2]: chaoscon_
argv[3]: 1
argv[4]: 1262919782
argv[5]: jer
argv[6]: 192.168.0.250
argv[7]: +i
argv[8]: DAqAD6
argv[9]: ABAAD
**/

/* [Debug 2] RECV: AB N chaoscon_ 1 1262920107 jer 192.168.0.250 DAqAD6 ABAAE :J. Johnston - from connected server (argv: 10) */
/**
argc: 10
argv[0]: AB
argv[1]: N
argv[2]: chaoscon_
argv[3]: 1
argv[4]: 1262920107
argv[5]: jer
argv[6]: 192.168.0.250
argv[7]: DAqAD6
argv[8]: ABAAE
argv[9]: J. Johnston
**/

/* [Debug 2] RECV: ABAAE N ph33r 1262920168 - nick change (arg) */
static void omega_nick  (Link *source, int argc, char **argv)
{
    User    *u;
    int     i;

    if (argc == 11)
        AddUser (argv[2], argv[5], argv[6], argv[10], source->sid, argv[7], argv[9], NULL, NULL);
    if (argc == 10)
        AddUser (argv[2], argv[5], argv[6], argv[9], source->sid, NULL, argv[8], NULL, NULL);
        
    return;
    fprintf (stderr, "argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        fprintf (stderr, "argv[%d]: %s\n", i, argv[i]);
}

static void omega_b     (Link *source, int argc, char **argv)
{
    Channel     *c;
    User        *u;

    int         i;

    if (argc < 5)
        return;

    if ((c = find_channel (argv[2])) == NULL)
    {
        c               = new_chan (argv[2]);
        c->channelts    = atol(argv[3]);
    }
    else
    {
        if (atol(argv[3]) < c->channelts)
            c->channelts    = atol(argv[3]);
    }

    printf ("argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        printf ("argv[%d]: %s\n", i, argv[i]);
}

static void omega_squit     (Link *source, int argc, char **argv)
{
    if (!source)
        return;

    exit_serv (argv[2]);
    return;
}

static void omega_join  (User *source, int argc, char **argv)
{
    int     i;

    printf ("argc: %d\n", argc);
    for (i = 0; i < argc; i++)
        printf ("argv[%d]: %s\n", i, argv[i]);
}

static void omega_quit  (User *source, int argc, char **argv)
{
    if (!source)
        return;

    exit_user (source->nick);
    return;
}

static void omega_privmsg    (User *source, int argc, char **argv)
{
    char    *params[309], buf[512];
    int     n, x;
    User    *target;

    for (n = 0; n <= 309; n++)
        params[n] = NULL;

    if (!source || (argc < 4) || (*argv[2] == '#'))
        return;

    if ((target = find_user (argv[2])) == NULL)
        return;

    n   = generictoken (' ', sizeof(params), argv[3], params);

    if (n > 1)
        ExpandParv (buf, sizeof(buf), 1, n, params);

    x   = do_cmd (params[0], target, source, n, buf);

    if (x == 1)
    {
        sendto_one (target, source, "%s", CmdErr(x));
        return;
    }

    return;
}
