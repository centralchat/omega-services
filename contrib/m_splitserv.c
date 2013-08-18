/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2009-2012, Jeremy Johnston <jeremy at ssnet dot ca>
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *    $Id: m_splitserv.c 2394 2012-06-30 15:56:22Z twitch $
 */

/*
 * This module provides similar functionality as SplitServ
 * found in Sentinel - IRC Statistical and Operator Services.
 */

/*
 * INFO:
 *  This module now uses the internal database.
 *
 * CONFIG:
 * <splitserv nick="SplitServ" ident="splitserv" host="splits.services" chan="#splits" timeout="2d" splitsave="no" splitexpire="yes">
 *
 * BUGS:
 *
 */

#include "stdinc.h"
#include "server.h"

#define NAME    "m_splitserv"
#define AUTHOR  "Jer"
#define VERSION "$Id: m_splitserv.c 2394 2012-06-30 15:56:22Z twitch $"

struct SplitConfig {
    char    nick[30];
    char    ident[15];
    char    host[64];
    char    chan[30];

    int     splitsave;
    int     splitexpire;
    time_t  splitexpiretimeout;
} SplitConfig;

/* STOP STOP STOP STOP STOP STOP
 * Do not edit below this line unless you know what
 * you are doing!!!
 */

#define TIMEBUF		100

/* These items are for the timediff() function, used to process time
** quantities (3 days, 45 minutes, 21 seconds)
*/
#define T_DAY       "day"
#define T_DAYS      "days"
#define T_HOUR      "hour"
#define T_HOURS     "hours"
#define T_MINUTE    "minute"
#define T_MINUTES   "minutes"
#define T_SECOND    "second"
#define T_SECONDS   "seconds"

#define BUFSIZE     512

/* Init/Deinit */
int     splitserv_init ();
void    splitserv_deinit ();

/* Commands */
static void splitserv_cmd_split     (User *, int, char **);
static void splitserv_cmd_forget    (User *, int, char **);
static void splitserv_cmd_add       (User *, int, char **);

static void splitserv_cmd_privmsg   (User *, int, char **);

static void splitserv_show_splits   (char *);
static void splitserv_show_map      (char *);
static struct SplitServer *splitserv_addsplit      (char *, char *, int);
static void splitserv_forgetsplit   (char *);

struct SplitServer  *splitserv_findsplit    (char *);

/* Event Handlers */
int    splitserv_servconnect   (int, void *);
int    splitserv_servexit      (int, void *);
int    splitserv_expiresplits  (int, void *);
int    splitserv_rehash	       (int, void *);

/* Split Tracking */
struct SplitServer {
    char    servname[63];
    char    uplink[63];

    time_t  splittime;
};

dlink_list  splits;     /* Linked list of split servers (struct *SplitServer) */

int     splitserv_savedb    (int, void*);
void    splitserv_loaddb    (void);

MODHEADER (NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7), splitserv_init, splitserv_deinit);

User *s_SplitServ = NULL;

int splitserv_init ()
{
    Channel     *chan;
    char	*ce;
    int         ch, ret;

    splits.head = splits.tail = NULL;

    /* Configuration Defaults */
    strlcpy (SplitConfig.nick, "SplitServ", sizeof(SplitConfig.nick));
    strlcpy (SplitConfig.ident, "splitserv", sizeof(SplitConfig.ident));
    strlcpy (SplitConfig.host, "split.services", sizeof(SplitConfig.host));
    strlcpy (SplitConfig.chan, "#splits", sizeof(SplitConfig.chan));

    SplitConfig.splitsave           = 0;
    SplitConfig.splitexpire         = 1;
    SplitConfig.splitexpiretimeout  = 172800;

    /* Read in Configuration */
    if ((ce = (char *) get_config_entry ("splitserv", "nick")))
        strlcpy (SplitConfig.nick, ce, sizeof(SplitConfig.nick));
    if ((ce = (char *) get_config_entry ("splitserv", "ident")))
        strlcpy (SplitConfig.ident, ce, sizeof(SplitConfig.ident));
    if ((ce = (char *) get_config_entry ("splitserv", "host")))
        strlcpy (SplitConfig.host, ce, sizeof(SplitConfig.host));
    if ((ce = (char *) get_config_entry ("splitserv", "channel")))
        strlcpy (SplitConfig.chan, ce, sizeof(SplitConfig.chan));
    if ((ce = (char *) get_config_entry ("splitserv", "timeout")))
        SplitConfig.splitexpiretimeout  = time_to_sec(ce);

    if ((ret = get_config_bool ("splitserv", "splitsave", 0)) >= 0)
        SplitConfig.splitsave   = ret;
    if ((ret = get_config_bool ("splitserv", "splitexpire", 0)) >= 0)
        SplitConfig.splitexpire = ret;

    if (s_SplitServ != NULL)
        return MOD_STOP;

    if (!(s_SplitServ = NewClient (SplitConfig.nick, SplitConfig.ident, SplitConfig.host, "Split Tracking Services")))
        return MOD_STOP;

	
	//Tell fantasy handler to ignore this client. - Twitch
	s_SplitServ->fantasy = 0;

    if (!(chan = find_channel (SplitConfig.chan)))
    {
        chan            = new_chan (SplitConfig.chan);
        chan->channelts = time(NULL);
    }

    AddToChannelU (s_SplitServ, chan);

    splitserv_loaddb ();

    AddEvent ("SERVERCONN", splitserv_servconnect);
    AddEvent ("SERVEREXIT", splitserv_servexit);
    AddEvent ("REHASH", splitserv_rehash);
    AddEvent ("SAVEDB", splitserv_savedb);

    ch  = AddCmd (s_SplitServ, "SPLIT", ACC_FLAG_OPER, splitserv_cmd_split, NULL, 0);
    ch  = AddCmd (s_SplitServ, "FORGET", ACC_FLAG_OPER, splitserv_cmd_forget, NULL, 0);
    ch  = AddCmd (s_SplitServ, "ADD", ACC_FLAG_OPER, splitserv_cmd_add, NULL, 0);

    AddUserCmd ("PRIVMSG", splitserv_cmd_privmsg);

    AddTimedEvent ("Split expire", splitserv_expiresplits, 5);

    return MOD_CONT;
}

void splitserv_deinit ()
{
    struct SplitServer  *sp;
    dlink_node          *dl, *tdl;

    DelEvent ("SERVERCONN", splitserv_servconnect);
    DelEvent ("SERVEREXIT", splitserv_servexit);
    DelEvent ("REHASH", splitserv_rehash);
    DelEvent ("SAVEDB", splitserv_savedb);

    DelTimedEvent ("Split expire");

    splitserv_savedb (0, NULL);

    if (s_SplitServ) {
        DelCmd (s_SplitServ, "SPLIT", splitserv_cmd_split, NULL);
        DelCmd (s_SplitServ, "FORGET", splitserv_cmd_forget, NULL);
        DelCmd (s_SplitServ, "ADD", splitserv_cmd_add, NULL);

        DelUserCmd ("PRIVMSG", splitserv_cmd_privmsg);

        DelClient (s_SplitServ->nick);
    }

    /* Be sure to release all split entries */
    DLINK_FOREACH_SAFE (dl, tdl, splits.head)
    {
        sp  = (struct SplitServer *) dl->data;

        dlink_delete (dl, &splits);
        dlink_free (dl);
        free (sp);
    }

    alog (LOG_DEBUG, "SplitServ: Module unloading");
}

void splitserv_loaddb (void)
{
    struct SplitServer	*sp;
    dlink_node		*dl, *tdl;

    DBContainer *dc	= NULL;
    DBEntry	*de;

    int 	ac, i;
    char	*av[5];
    char	*value;

    if (!SplitConfig.splitsave) {
        alog (LOG_INFO, "Split saving is disabled");
        return;
    }

    if ((!(dc = get_db_rows("SPLIT")))) {
        alog (LOG_DEBUG, "No splits found in database");
        return;
    }

    /* Initialize the array */
    for (i = 0; i < 5; i++)
        av[i]	= '\0';

    DLINK_FOREACH_SAFE (dl, tdl, dc->entries.head)
    {
        de	= dl->data;
	value	= strdup(de->value);

	ac	= generictoken (' ', 5, value, av);

        alog (LOG_DEBUG, "SplitServ: Adding %s to the splits table", de->key);

        if (ac < 2) {
            alog (LOG_INFO, "SplitServ: Invalid database entry for %s, removing", de->key);
            del_db_entry ("SPLIT", de->key);

            if (value)
                free (value);
            continue;
        }

	sp	= splitserv_addsplit (de->key, av[0], atoi(av[1]));

        if (value)
            free(value);
    }

    return;
}

int splitserv_savedb (int ac, void *nil)
{
    struct SplitServer  *sp;

    dlink_node          *dl, *tdl;
    
    DBContainer *dc     = NULL;
    DBEntry     *de;

	if ((dc = get_db_rows ("SPLIT"))) {
		DLINK_FOREACH_SAFE (dl, tdl, dc->entries.head) {
			de      = dl->data;

			del_db_entry ("SPLIT", de->key);
		}
	}


    if (!SplitConfig.splitsave) {
        alog (LOG_INFO, "Split saving is disabled");
        return;
    }

    DLINK_FOREACH (dl, splits.head)
    {
        sp = (struct SplitServer *) dl->data;

	alog (LOG_DEBUG, "Adding split %s -> %s (%d) to database", sp->servname, sp->uplink, (int) sp->splittime);
        add_db_entry ("SPLIT", sp->servname, "%s %d", sp->uplink, (int) sp->splittime);
    }

    return;
}

int splitserv_servconnect (int unused, void *server)
{
    Link *serv = server;
    dlink_node          *dl, *tdl;
    struct SplitServer  *sp;

    if ((strcasecmp (serv->name, CfgSettings.servername)) == 0) /* Ignore ourself */
        return EVENT_OK;

    DLINK_FOREACH_SAFE (dl, tdl, splits.head)
    {
        sp  = (struct SplitServer *) dl->data;

        if (strcasecmp (sp->servname, serv->name) == 0)
        {
            sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "END OF SPLIT: %s to %s: %s", sp->servname, sp->uplink, timediff(time(NULL) - sp->splittime));

            free (sp);
            dlink_delete (dl, &splits);
            dlink_free (dl);
            return EVENT_STOP;
        }
    }

    sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "CONNECT: %s to %s", serv->name, (serv->uplink)? serv->uplink->name : "unknown");

    return EVENT_OK;
}

int splitserv_servexit (int unused, void *server)
{
    Link *serv = server;
    dlink_node          *dl;
    struct SplitServer  *sp;

    if ((strcasecmp (serv->name, CfgSettings.servername)) == 0) /* Ignore ourself */
        return EVENT_CONT;

    sp  = (struct SplitServer *) malloc (sizeof(struct SplitServer));
    dl  = dlink_create ();

    strlcpy (sp->servname, serv->name, sizeof(sp->servname));

    if (serv->uplink)
        strlcpy (sp->uplink, serv->uplink->name, sizeof(sp->uplink));
    else
        strlcpy (sp->uplink, "null", sizeof(sp->uplink));

    sp->splittime   = time(NULL);

    sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "SPLIT: %s from %s", sp->servname, sp->uplink);

    dlink_add_tail (sp, dl, &splits);
    return EVENT_OK;
}

int splitserv_expiresplits (int ac, void *nil)
{
    struct SplitServer  *sp;
    dlink_node          *dl, *tdl;

    if (!SplitConfig.splitexpire)
        return EVENT_OK;

    DLINK_FOREACH_SAFE (dl, tdl, splits.head)
    {
        sp  = (struct SplitServer *) dl->data;

        if ((time(NULL) - sp->splittime) > SplitConfig.splitexpiretimeout)
        {
            sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "SPLIT EXPIRED AFTER %s: %s", timediff(time(NULL) - sp->splittime), sp->servname);
            del_db_entry ("SPLIT", sp->servname);

            dlink_delete (dl, &splits);
            dlink_free (dl);

            free (sp);
        }
    }
    return EVENT_OK;
}

int splitserv_rehash (int ac, void *nil)
{
	char	*ce;
	char	channame[30];
	Channel	*channel;
	int	ret, splitexpire;

	dlink_node	*dl, *tdl;
	DBContainer	*dc	= NULL;
	DBEntry		*de;

	strlcpy (channame, SplitConfig.chan, sizeof(channame));
	splitexpire	= SplitConfig.splitexpire;

	if ((ce = (char *) get_config_entry ("splitserv", "nick")))
		strlcpy (SplitConfig.nick, ce, sizeof(SplitConfig.nick));
	if ((ce = (char *) get_config_entry ("splitserv", "ident")))
		strlcpy (SplitConfig.ident, ce, sizeof(SplitConfig.ident));
        if ((ce = (char *) get_config_entry ("splitserv", "host")))
		strlcpy (SplitConfig.host, ce, sizeof(SplitConfig.host));
	if ((ce = (char *) get_config_entry ("splitserv", "channel")))
		strlcpy (SplitConfig.chan, ce, sizeof(SplitConfig.chan));
	if ((ce = (char *) get_config_entry ("splitserv", "timeout")))
		SplitConfig.splitexpiretimeout	= time_to_sec(ce);
	if ((ret = get_config_bool ("splitserv", "splitsave", 0)) >= 0)
		SplitConfig.splitsave	= ret;
	if ((ret = get_config_bool ("splitserv", "splitexpire", 0)) >= 0)
		SplitConfig.splitexpire	= ret;

	alog (LOG_DEBUG, "SplitServ: splitsave: %d", SplitConfig.splitsave);

	if (!SplitConfig.splitsave) {
		/* Flush all our database entries */
		if ((dc = get_db_rows ("SPLIT"))) {
			DLINK_FOREACH_SAFE (dl, tdl, dc->entries.head) {
				de	= dl->data;

				del_db_entry ("SPLIT", de->key);
			}
		}
	}

	if (!s_SplitServ)
		return EVENT_STOP;

	if ((strcasecmp(s_SplitServ->nick, SplitConfig.nick) != 0) || (strcasecmp(s_SplitServ->user, SplitConfig.ident) != 0) || (strcasecmp(s_SplitServ->host, SplitConfig.host) != 0)) {
		DelCmd (s_SplitServ, "SPLIT", splitserv_cmd_split, NULL);
		DelCmd (s_SplitServ, "FORGET", splitserv_cmd_forget, NULL);
		DelCmd (s_SplitServ, "ADD", splitserv_cmd_add, NULL);

		DelUserCmd ("PRIVMSG", splitserv_cmd_privmsg);

		DelClient (s_SplitServ->nick);

		/* Re-introduce our client */
		if ((s_SplitServ = NewClient (SplitConfig.nick, SplitConfig.ident, SplitConfig.host, "Split Tracking Services"))) {
			if (!(channel = find_channel (SplitConfig.chan))) {
				channel			= new_chan (SplitConfig.chan);
				channel->channelts	= time(NULL);
			}

			AddToChannelU (s_SplitServ, channel);

			AddCmd (s_SplitServ, "SPLIT", ACC_FLAG_OPER, splitserv_cmd_split, NULL, 0);
			AddCmd (s_SplitServ, "FORGET", ACC_FLAG_OPER, splitserv_cmd_forget, NULL, 0);
			AddCmd (s_SplitServ, "ADD", ACC_FLAG_OPER, splitserv_cmd_add, NULL, 0);

			AddUserCmd ("PRIVMSG", splitserv_cmd_privmsg);
		}
	} else {
		if ((strcasecmp(channame, SplitConfig.chan) != 0)) {
			if (!(channel  = find_channel (channame)))
				return EVENT_CONT;

			channel	= find_channel (channame);	// Old Channgel
			ircd_part (s_SplitServ, channel, "Split reporting channel changed.");
			DelFromChannel (s_SplitServ, channel);

			/* Find/Create new channel */
			if (!(channel = find_channel (SplitConfig.chan))) {
				channel			= new_chan (SplitConfig.chan);
				channel->channelts	= time(NULL);
			}

			AddToChannelU (s_SplitServ, channel);
		}
	}
    
	return EVENT_OK;
}

static void splitserv_cmd_privmsg (User *source, int argc, char **argv)
{
    char    *av[5];
    int     ac;

    if (!source)
        return;

    if ((*argv[1] != '#') || (*argv[2] != '!'))
        return;

    if ((strcasecmp (SplitConfig.chan, argv[1])) != 0)
        return;

    ac  = generictoken (' ', 5, argv[2], av);

    av[0]++;    // Skip the fantasy character

    if ((strcasecmp (av[0], "SPLIT")) == 0)
    {
        splitserv_show_splits (argv[1]);
        return;
    }

    if ((strcasecmp (av[0], "ADD")) == 0)
    {
        if (ac >= 3)
            splitserv_addsplit (av[1], av[2], 0);
        return;
    }

    if ((strcasecmp (av[0], "FORGET")) == 0)
    {
        if (ac >= 2)
            splitserv_forgetsplit (av[1]);
        return;
    }

    if ((strcasecmp (av[0], "MAP")) == 0)
    {
        splitserv_show_map (argv[1]);
        return;
    }
}

static void splitserv_cmd_split (User *user, int ac, char **av)
{
    splitserv_show_splits (user->nick);
}

static void splitserv_cmd_forget (User *user, int ac, char **av)
{
    if (!av[0])
        return;
    splitserv_forgetsplit (av[0]);
}

static void splitserv_cmd_add (User *user, int ac, char **av)
{
    if (!av[0] || !av[1])
        return;

    splitserv_addsplit (av[0], av[1], 0);
}

static void splitserv_show_splits (char *target)
{
    dlink_node          *dl;
    struct SplitServer  *sp;
    int                 cnt = 0;

    Channel      *c = NULL;
    User         *u = NULL;

    if (*target == '#')
        c = find_channel (target);
    else
        u = find_user (target);


    if (!c && !u)
        return;

    DLINK_FOREACH (dl, splits.head)
    {
        sp = (struct SplitServer *) dl->data;

        cnt++;

        if (c)
            sendto_channel (s_SplitServ, c, "SPLIT: %s from %s: %s", sp->servname, sp->uplink, timediff(time(NULL) - sp->splittime));
        else
            sendto_one (s_SplitServ, u, "SPLIT: %s from %s: %s", sp->servname, sp->uplink, timediff(time(NULL) - sp->splittime));
    }

    if (cnt == 0) {
        if (c)
            sendto_channel (s_SplitServ, c, "NO SPLITS");
        else
            sendto_one (s_SplitServ, u, "NO SPLITS");
    }

    return;
}

static void splitserv_show_map (char *target)
{
    return;
}

static struct SplitServer *splitserv_addsplit (char *server, char *uplink, int ts)
{
    struct SplitServer  *sp;
    Link                *serv;
    dlink_node          *dl;

    if ((sp = splitserv_findsplit (server)) != NULL)
        return NULL;

    if ((serv = find_serv (server)) != NULL)
        return NULL;

    sp  = (struct SplitServer *) malloc (sizeof (struct SplitServer));
    dl  = dlink_create ();

    strlcpy (sp->servname, server, sizeof(sp->servname));
    strlcpy (sp->uplink, uplink, sizeof(sp->uplink));

    if (ts == 0)
        sp->splittime   = time(NULL);
    else
        sp->splittime   = ts;

    dlink_add_tail (sp, dl, &splits);

    sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "SPLIT: %s from %s", sp->servname, sp->uplink);
    add_db_entry ("SPLIT", sp->servname, "%s %d", sp->uplink, (int) sp->splittime);

    return sp;
}

static void splitserv_forgetsplit (char *server)
{
    struct SplitServer  *sp;
    dlink_node          *dl, *tdl;

    if ((sp = splitserv_findsplit (server)) == NULL)
        return;

    sendto_channel (s_SplitServ, find_channel(SplitConfig.chan), "FORGOT SPLIT: %s", sp->servname);
    del_db_entry ("SPLIT", sp->servname);

    dl = dlink_find_delete (sp, &splits);

    dlink_free (dl);
    free (sp);

}

struct SplitServer *splitserv_findsplit (char *server)
{
    struct SplitServer  *sp;
    dlink_node          *dl;

    DLINK_FOREACH (dl, splits.head)
    {
        sp  = (struct SplitServer *) dl->data;

        if ((strcasecmp (sp->servname, server)) == 0)
            return sp;
    }

    return NULL;
}

/* print a period of seconds into human readable format */
char *timediff(time_t u)
{
  static char buf[TIMEBUF];

  if (u > 86400)
  {
    time_t days = u / 86400;

    snprintf(buf, TIMEBUF, "%ld %s, %02ld:%02ld",
             (long)days, (days == 1) ? T_DAY : T_DAYS,
             (long)((u / 3600) % 24), (long)((u / 60) % 60));
  }
  else if (u > 3600)
  {
    time_t hours = u / 3600;
    time_t minutes = (u / 60) % 60;

    snprintf(buf, TIMEBUF, "%ld %s, %ld %s",
             (long)hours, (hours == 1) ? T_HOUR : T_HOURS, (long)minutes,
             (minutes == 1) ? T_MINUTE : T_MINUTES);
  }
  else
  {
    time_t minutes = u / 60;
    time_t seconds = u % 60;

    snprintf(buf, TIMEBUF,
             "%ld %s, %ld %s",
             (long)minutes, (minutes == 1) ? T_MINUTE : T_MINUTES,
             (long)seconds, (seconds == 1) ? T_SECOND : T_SECONDS);
  }

  return buf;
}
