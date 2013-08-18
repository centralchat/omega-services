/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
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
 *    $Id: g_watch.c 2394 2012-06-30 15:56:22Z twitch $
 */


#include "stdinc.h"
#include "server.h"

#define NAME	"m_watch"
#define AUTHOR	"Jer"
#define VERSION "$Id: g_watch.c 2394 2012-06-30 15:56:22Z twitch $"
#define BUFSIZE	512

/* Actions */
#define ACTION_NONE		0
#define ACTION_KICK		1
#define ACTION_KILL		2
#define ACTION_GLINE		3

/* Options */
#define OPTION_IGNOREOPER	0x02
#define OPTION_IGNOREACCESS	0x04
#define OPTION_REGEX		0x08

dlink_list	watchlist;
dlink_list	watchqueue;

int		bantime;
int		defaultflags;
int		unloading;

struct watch_
{
	Channel		*channel;
	int		action;
	int		options;
	char		message[MAXLEN + 1];
	char		nickpattern[512];

	char		channame[512];
	int		existing;

	int		bantime;
	int		config;
};

struct watchqueue_
{
	User		*user;
	struct watch_	*watch;
};

struct watchaction_
{
	int	action;
	char	*name;
};

struct watchaction_ actions[15] = {
	{ ACTION_NONE,	"none"		},
	{ ACTION_KICK,	"kick"		},
	{ ACTION_KICK,	"kickban"	},
	{ ACTION_KILL,	"kill"		},
	{ ACTION_GLINE,	"gline"		},
	{ ACTION_GLINE,	"zline" 	},
	{ ACTION_GLINE, "gzline"	},
	{ ACTION_GLINE,	"kline"		},
	{ 0, NULL }
};

/* Event Handler for channel joins */
static int		evh_chan_join	(int, void*);
static int		evh_user			(int, void*);
static int		evh_watchqueue(int, void*);

/* Our local routines */
static void		load_watchdb	(void);
static int		load_watch		(int, void*);
static int		save_watchdb	(int, void*);
static int		find_action		(char *);
static int		take_action		(User *, struct watch_ *);
static char		*action_num_to_name	(int);
struct watch_		*is_watched 		(char *);

static struct watch_	*add_watch		(User *, char *, char *, char *, int, int, int);
static void		del_watch		(char *);

/* Command handler */
static void		cmd_watch	(User *, int ac, char **);

/* Help Handlers */
static void		help_watch	(User *);
static void		help_watch_ext	(User *);

int Module_Init();
void Module_Close();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7),Module_Init, Module_Close);


int Module_Init()
{
	int	ch, cnt, n;
	char	*ce;
	const char	*list[30];

	watchlist.head = watchlist.tail = NULL;

	if (s_Guardian)	{
		ch = AddCmd (s_Guardian, "WATCH", ACC_FLAG_OPER, cmd_watch, help_watch_ext, 1);
		AddHelp    (s_Guardian, help_watch);
	} 
	else 
		throwModErr("Module Error, Guardian core not initialized.","");

	/* Initialize defaults */
	bantime		= 0;
	defaultflags	= 0x0;
	unloading	= FALSE;

	if ((ce = (char *) get_config_entry ("watch", "bantime")))
		bantime	= time_to_sec(ce);

	if ((cnt = get_config_list ("watch", "defaultflags", ',', list)) > 0) {
		for (n = 0; n < cnt; n++) {
			if (!list[n])
				return;

			if ((strcasecmp(list[n], "IGNOREOPERS")) == 0)
				defaultflags	|= OPTION_IGNOREOPER;
			if ((strcasecmp(list[n], "IGNOREACCESS")) == 0)
				defaultflags	|= OPTION_IGNOREACCESS;
			if ((strcasecmp(list[n], "REGEX")) == 0)
				defaultflags	|= OPTION_REGEX;
		}

	}

	load_watchdb ();
	load_watch   (0, NULL);

	AddTimedEvent ("WatchQueue", evh_watchqueue, 30);

	AddEvent ("REHASH", load_watch);
	AddEvent ("CHANJOIN", evh_chan_join);

	AddEvent ("USERCONN", evh_user);
	AddEvent ("SAVEDB", save_watchdb);

	return MOD_CONT;
}

void Module_Close()
{
	dlink_node	*dl, *tdl;
	struct watch_	*watch;
	Channel		*channel;

	DelTimedEvent ("WatchQueue");
	
	DelEvent ("CHANJOIN", evh_chan_join);
	DelEvent ("USERCONN", evh_user);
	DelEvent ("SAVEDB", save_watchdb);
	DelEvent ("REHASH", load_watch);

	save_watchdb (0, NULL);

	if (s_Guardian)
		DelCmd (s_Guardian, "WATCH", cmd_watch, help_watch_ext);

	// Set unloading
	unloading	= TRUE;

	/* Free memory we have allocated */
	DLINK_FOREACH_SAFE (dl, tdl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		del_watch ((*watch->nickpattern == '\0')? watch->channame : watch->nickpattern);
	}

	return;
}

static void load_watchdb (void)
{
	Channel			*channel;
	ChanUser		*cu;
	User			*user;
	struct watch_		*watch;
	dlink_node		*dl, *tdl, *wdl, *cdl, *ctdl, *udl, *utdl;

	DBContainer		*dc;
	DBEntry			*de;

	char			*av[15];
	char			reason[MAXLEN];
	char			*val;
	int			ac, i;

	watchlist.head = watchlist.tail = NULL;

	print_db_entries ();

	if ((!(dc = get_db_rows ("WATCH")))) {
		alog (LOG_DEBUG, "Watch: load_watchdb(): No WATCH entries returned from database");
		return;
	}

	DLINK_FOREACH_SAFE (dl, tdl, dc->entries.head)
	{
		de	= (DBEntry *) dl->data;

		if (!de->key || !de->value) {
			alog (LOG_DEBUG2, "Watch: load_watchdb(): Invalid WATCH database entry");
			continue;
		}

		val	= strdup(de->value);
		ac	= generictoken (' ', 15, val, av);

		alog (LOG_DEBUG2, "Watch: load_watchdb(): value: %s", de->value);

		if (ac < 3) {
			alog (LOG_DEBUG2, "Watch: load_watchdb(): Too few arguments for %s: %d expecting =>3", de->key, ac);
			if (val)
				free (val);
			continue;
		}

		for (i = 0; i < ac; i++)
			alog (LOG_DEBUG2, "Watch: load_watchdb(): av[%d]: %s", i, av[i]);

		if (!isalpha(*av[2])) {
			if (ac < 4) {
				alog (LOG_DEBUG2, "Watch: load_watchdb(): Too few arguments for %s: %d expecting =>4", de->key, ac);
				if (val)
					free (val);
				continue;
			}

			ExpandParv (reason, sizeof(reason), 3, ac, av);

			if (!(watch = add_watch (NULL, de->key, av[0], reason, atoi(av[1]), atoi(av[2]), FALSE))) {
				alog (LOG_DEBUG, "Watch: load_watchdb(): Failed to add watch pattern %s", de->key);
				if (val) free (val);
				continue;
			}

		} else {
			ExpandParv (reason, sizeof(reason), 2, ac, av);

			if (!(watch = add_watch (NULL, de->key, av[0], NULL, atoi(av[1]), 0, FALSE))) {
				alog (LOG_DEBUG, "Watch: load_watchdb(): Failed to add watch pattern %s", de->key);
				if (val) free (val);
				continue;
			}
		}

                if (!watch->config)
                        add_db_entry ("WATCH", (*watch->nickpattern != '\0')? watch->nickpattern : watch->channame, "%d %d %d %s",
                        watch->action, watch->options, watch->bantime, watch->message);

		if (val)
			free (val);

		print_db_entries ();
	}

	return;
}

static int save_watchdb (int ac, void *nil)
{
	struct watch_		*watch;
	dlink_node		*dl, *tdl;
	DBContainer		*dc;
	DBEntry			*de;

	DLINK_FOREACH_SAFE (dl, tdl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		if ((!watch) || (!watch->message))
			continue;
		if (watch->config) {	// Only save dynamic entries to the database
			alog (LOG_DEBUG2, "Watch: save_watchdb(): Skipping pattern %s", (*watch->channame != '\0')? watch->channame : watch->nickpattern);
			continue;
		}

		alog (LOG_DEBUG2, "Watch: save_watchdb(): Attempting to save pattern: %s", (*watch->channame != '\0')? watch->channame : watch->nickpattern);

		if (watch->nickpattern[0] != '\0') {
			alog (LOG_DEBUG, "Watch: save_watchdb(): Saving nick pattern %s (Action: %d) (Message: %s)", watch->nickpattern, watch->action, watch->message);
			add_db_entry ("WATCH", watch->nickpattern, "%d %d %d %s", watch->action, watch->options, watch->bantime, watch->message);
		} else {
			if (watch->channame[0] == '\0')
				continue;
			alog (LOG_DEBUG, "Watch: save_watchdb(): Saving channel %s (Action: %d) (Message: %s)", watch->channame, watch->action, watch->message);
			add_db_entry ("WATCH", watch->channame, "%d %d %d %s", watch->action, watch->options, watch->bantime, watch->message);
		}

		print_db_entries ();
	}

	return 1;
}

static int load_watch (int ac, void * nil)
{
	int		cnt, n, options;
	char		*list[200];
	char		*flags;

	dlink_list	*watch_entries;
	dlink_node	*dl, *tdl;
	ConfBase	*cb;
	ConfEntry	*ce;
	int		aindex, bindex, cindex, dindex, eindex;

	struct watch_	*watch;

	/* To reload watches properly from the config, we need to first remove them */
	DLINK_FOREACH_SAFE (dl, tdl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		if (watch->config)
			del_watch ((*watch->nickpattern == '\0')? watch->channame : watch->nickpattern);
	}

	if (!(watch_entries = get_config_base ("watchentry")))
		return;

	aindex	= HASH("pattern", HASH_B);
	bindex	= HASH("action",  HASH_B);
	cindex  = HASH("reason",  HASH_B);
	dindex  = HASH("flags",   HASH_B);
        eindex	= HASH("bantime", HASH_B);

	DLINK_FOREACH (dl, watch_entries->head)
	{
		cb	= (ConfBase *) dl->data;

		if (!cb->map[aindex] || !cb->map[bindex] || !cb->map[cindex])
			continue;

		if ((watch = is_watched(cb->map[aindex])))
			continue;

		options	= 0x0;

		if ((cb->map[dindex])) {
			flags	= strdup(cb->map[dindex]);
			cnt	= generictoken(',', 50, flags, list);

			for (n = 0; n < cnt; n++) {
				if ((strcasecmp(list[n], "IGNOREOPERS")) == 0)
					options	|= OPTION_IGNOREOPER;
				else if ((strcasecmp(list[n], "IGNOREACCESS")) == 0)
					options |= OPTION_IGNOREACCESS;
				else if ((strcasecmp(list[n], "REGEX")) == 0)
					options |= OPTION_REGEX;
			}

			if (flags)
				free (flags);
		}

		add_watch (NULL, cb->map[aindex], cb->map[bindex], cb->map[cindex],
			options,
			(cb->map[eindex])? time_to_sec(cb->map[eindex]) : 0,
			TRUE);
	}
}

static struct watch_ *add_watch (User *user, char *pattern, char *action, char *reason, int options, int bantime, int config)
{
	struct watch_	*watch;
	dlink_node	*dl, *tdl, *wdl;
	Channel		*channel;
	ChanUser	*cu;

	if ((watch = is_watched(pattern))) {
		alog (LOG_DEBUG, "Watch: add_watch(): Attempted to add duplicate pattern %s", pattern);
		return watch;
	}

	watch	= (struct watch_ *) malloc (sizeof(struct watch_));
	wdl	= dlink_create ();

	watch->action	= (!isalpha(*action))? atoi(action) : find_action (action);
	watch->config	= config;
	watch->bantime	= bantime;
	watch->existing	= FALSE;

	if (options)
		watch->options	= options;
	else
		watch->options	= defaultflags;

	if (!reason)
		strlcpy (watch->message, "Watched Pattern", sizeof(watch->message));
	else
		strlcpy (watch->message, reason, sizeof(watch->message));

	if (pattern[0] == '#') {
		if (!(channel = find_channel (pattern)))
			channel	= new_chan (pattern);

		if (!channel) {
			free       (watch);
			dlink_free (wdl);
			return NULL;
		}

		watch->channel		= channel;
		watch->nickpattern[0]	= '\0';

		strlcpy (watch->channame, channel->name, sizeof(watch->channel));

		if (!in_channel (s_Guardian, channel)) {
			AddToChannelU (s_Guardian, channel);

			if (sync_state != BURSTING)
				ircd_join (s_Guardian, channel);
		} else
			watch->existing	= TRUE;

		DLINK_FOREACH_SAFE (dl, tdl, channel->users.head) {
			cu	= (ChanUser *) dl->data;

			take_action (cu->user, watch);
		}

	} else {
		strlcpy (watch->nickpattern, pattern, sizeof(watch->nickpattern));

		watch->channel		= NULL;
		watch->channame[0]	= '\0';
		watch->options		|= OPTION_IGNOREOPER;

		if (watch->action == ACTION_KICK) {
			watch->action	= ACTION_KILL;

			if (user)
				sendto_one (s_Guardian, user, "\002WARNING\002 Action %s for nickname pattern translated to kill, kick/kickban can not be performed on nickname patterns.", action_num_to_name(watch->action));
			else
				alog (LOG_INFO, "Watch: add_watch(): Action %s for nickname pattern %s translated to kill", action_num_to_name(watch->action), watch->nickpattern);
		}

		DLINK_FOREACH_SAFE (dl, tdl, userlist.head) {
			user	= (User *) dl->data;

                        if (watch->options & OPTION_REGEX) {
				if ((match_regx (user->nick, watch->nickpattern)) == 0) {
                                        take_action (user, watch);
                                        continue;
                                }
                        } else {
                                if ((match_wild (watch->nickpattern, user->nick, 0))) {
                                        take_action (user, watch);
                                        continue;
                                }
                        }

                        alog (LOG_DEBUG3, "Watch: add_watch(): %s did not match any watch rule", user->nick);
		}
	}

	if (!config)
		add_db_entry ("WATCH", (*watch->nickpattern != '\0')? watch->nickpattern : watch->channame, "%d %d %d %s",
			watch->action, watch->options, watch->bantime, watch->message);

	dlink_add_tail (watch, wdl, &watchlist);
	return watch;
}

static void del_watch (char *pattern)
{
	struct watch_	*watch;
	dlink_node	*dl, *tdl;

	alog (LOG_DEBUG2, "Watch: del_watch(): Attempting to remove watch for %s", pattern);

	if (!(watch = is_watched (pattern)))
		return;

	DLINK_FOREACH_SAFE (dl, tdl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		if ((*watch->channame == '\0') && (*watch->nickpattern == '\0'))
			continue;

		if (*watch->channame != '\0') {
			if ((strcasecmp (watch->channame, pattern)) == 0) {
				if (!watch->config && !unloading)
					del_db_entry ("WATCH", watch->channame);

				if (watch->channel &&  in_channel(s_Guardian, watch->channel) && (!watch->existing)) {
					if (sync_state == IS_CON)
						ircd_part (s_Guardian, watch->channel, "Leaving, watch removed on channel");
					DelFromChannel (s_Guardian, watch->channel);
				}

				dlink_delete (dl, &watchlist);
				dlink_free   (dl);
				free         (watch);

				return;
			}
		} else {
			if ((strcasecmp (watch->nickpattern, pattern)) == 0) {
				if (!watch->config && !unloading)
					del_db_entry ("WATCH", watch->nickpattern);

				dlink_delete (dl, &watchlist);
				dlink_free   (dl);
				free         (watch);

				return;
			}
		}
	}
}

static void cmd_watch (User *source, int ac, char **params)
{
	int 	i, n, bantime;
	struct watch_	*watch;
	char			options[255];
	char			reason[MAXLEN];
	dlink_node		*dl, *tdl, *cdl, *ctdl, *udl, *tudl;
	Channel			*ch;
	ChanUser		*cu;
	User			*user;

	for (i = 0; i < ac; i++)
		alog (LOG_DEBUG, "Watch: cmd_watch() params[%d]: %s", i, params[i]);

	
	if ((strcasecmp (params[0], "LIST")) == 0)
	{
		sendto_one (s_Guardian, source, "Displaying channels/nickname patterns we currently are watching");

		DLINK_FOREACH (dl, watchlist.head)
		{
			watch	= (struct watch_ *) dl->data;

			options[0]	= '\0';

			// Build options
			if (watch->options & OPTION_IGNOREOPER)
				if (options[0] == '\0') { strlcpy (options, "IGNOREOPERS", sizeof(options)); } else { strlcat (options, ",IGNOREOPERS", sizeof(options)); }
			if (watch->options & OPTION_IGNOREACCESS)
				if (options[0] == '\0') { strlcpy (options, "IGNOREACCESS", sizeof(options)); } else { strlcat (options, ",IGNOREACCESS", sizeof(options)); }
			if (watch->options & OPTION_REGEX)
				if (options[0] == '\0') { strlcpy (options, "REGEX", sizeof(options)); } else { strlcat (options, ",REGEX", sizeof(options)); }

			if (watch->nickpattern[0] != '\0')
				sendto_one (s_Guardian, source, "%s [Action: %s] [Message: %s] [Options: %s] [Ban duration: %ds]%s", watch->nickpattern, action_num_to_name(watch->action), watch->message, options, watch->bantime, (watch->config)? " (config)" : "");
			else {
				if (watch->channame[0] == '\0')
					continue;
				sendto_one (s_Guardian, source, "%s [Action: %s] [Message: %s] [Options: %s] [Ban duration: %ds]%s", watch->channame, action_num_to_name(watch->action), watch->message, options, watch->bantime, (watch->config)? " (config)" : "");
			}
		}

		return;
	}
	else if ((strcasecmp (params[0], "HELP")) == 0)
	{
		help_watch_ext (source);
		return;
	}
	else if ((strcasecmp (params[0], "ADD")) == 0)
	{
		bantime		= 0;

		if (ac < 4)
		{
			sendto_one (s_Guardian, source, "Syntax: ADD <channel|nickname> <action> [time] <message>");
			return;
		}

		if (!isalpha(*params[3]) && (ac < 5)) {
			sendto_one (s_Guardian, source, "Syntax: ADD <channel|nickname> <action> [time] <message>");
			return;
		} else
			bantime	= time_to_sec(params[3]);

		if (is_watched (params[1]))
		{
			sendto_one (s_Guardian, source, "%s is already being watched", params[1]);
			return;
		}

		ExpandParv (reason, sizeof(reason), (bantime)? 4 : 3, ac, params);

		if (!(watch = add_watch (source, params[1], params[2], reason, 0, bantime, FALSE))) {
			sendto_one (s_Guardian, source, "Unable to add watch entry for %s", params[1]);
			return;
		}

		if (!watch->config)
			add_db_entry ("WATCH", (*watch->nickpattern != '\0')? watch->nickpattern : watch->channame, "%d %d %d %s",
			watch->action, watch->options, watch->bantime, watch->message);

		sendto_one (s_Guardian, source, "Added watch for %s (Action: %s Message: %s)", (watch->nickpattern[0] == '\0')? watch->channel->name : watch->nickpattern, action_num_to_name(watch->action), watch->message);

		return;
	}
	else if ((strcasecmp (params[0], "REMOVE")) == 0)
	{
		if (ac < 2)
			return;

		if (!is_watched (params[1]))
		{
			sendto_one (s_Guardian, source, "Channel %s is not being watched", params[1]);
			return;
		}

		del_watch (params[1]);
		sendto_one (s_Guardian, source, "No longer watching %s", params[1]);

		return;
	}
	else if ((strcasecmp (params[0], "CHANGE")) == 0)
	{
		sendto_one (s_Guardian, source, "WATCH CHANGE has been moved to WATCH SET");
		return;
	}
	else if ((strcasecmp (params[0], "SET")) == 0)
	{
		if (ac < 4) {
			sendto_one (s_Guardian, source, "Syntax: SET <option> <rule> <value>");
			return;
		}

		if (!(watch = is_watched (params[2]))) {
			sendto_one (s_Guardian, source, "%s is not currently being watched", params[2]);
			return;
		}

		if ((strcasecmp (params[1], "ACTION")) == 0) {
			watch->action	= (!isalpha(*params[3]))? atol(params[3]) : find_action(params[3]);

			if ((watch->nickpattern[0] != '\0') && (watch->action == ACTION_KICK)) {
				sendto_one (s_Guardian, source, "\002WARNING\002 Action for nickname pattern translated to kill, kick/kickban can not be performed on nickname patterns.");
				watch->action	= ACTION_KILL;
			}

			sendto_one (s_Guardian, source, "Action for %s has been set to \002%s\002", (watch->channel)? watch->channame : watch->nickpattern, action_num_to_name(watch->action));
			return;
		}
		else if ((strcasecmp (params[1], "MESSAGE")) == 0) {
			ExpandParv (watch->message, sizeof(watch->message), 3, ac, params);

			sendto_one (s_Guardian, source, "Message for %s has been set to \002%s\002", (watch->channel)? watch->channame : watch->nickpattern, watch->message);
			return;
		}
		else if ((strcasecmp (params[1], "IGNOREOPERS")) == 0) {
			if (((strcasecmp (params[3], "ON")) == 0) || ((strcasecmp (params[3], "YES")) == 0)) {
				if (!(watch->options & OPTION_IGNOREOPER)) {
					watch->options	|= OPTION_IGNOREOPER;
					sendto_one (s_Guardian, source, "IGNOREOPERS has been turned \002ON\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
				}
			} else {
				if (watch->options & OPTION_IGNOREOPER) {
					watch->options &= ~OPTION_IGNOREOPER;
					sendto_one (s_Guardian, source, "IGNOREOPERS has been turned \002OFF\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
				}
			}
		}
                else if ((strcasecmp (params[1], "IGNOREACCESS")) == 0) {
                        if (((strcasecmp (params[3], "ON")) == 0) || ((strcasecmp (params[3], "YES")) == 0)) {
                                if (!(watch->options & OPTION_IGNOREACCESS)) {
                                        watch->options  |= OPTION_IGNOREACCESS;
                                        sendto_one (s_Guardian, source, "IGNOREACCESS has been turned \002ON\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
                                }
                        } else {
                                if (watch->options & OPTION_IGNOREACCESS) {
                                        watch->options &= ~OPTION_IGNOREACCESS;
                                        sendto_one (s_Guardian, source, "IGNOREACCESS has been turned \002OFF\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
                                }
                        }
                }
                else if ((strcasecmp (params[1], "REGEX")) == 0) {
			if (watch->nickpattern[0] == '\0') {
				sendto_one (s_Guardian, source, "Option REGEX currently only applies to nickname patterns");
				return;
			}

                        if (((strcasecmp (params[3], "ON")) == 0) || ((strcasecmp (params[3], "YES")) == 0)) {
                                if (!(watch->options & OPTION_REGEX)) {
                                        watch->options  |= OPTION_REGEX;
                                        sendto_one (s_Guardian, source, "REGEX has been turned \002ON\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
                                }
                        } else {
                                if (watch->options & OPTION_REGEX) {
                                        watch->options &= ~OPTION_REGEX;
                                        sendto_one (s_Guardian, source, "REGEX has been turned \002OFF\002 for %s", (watch->channel)? watch->channame : watch->nickpattern);
                                }
                        }
                }
		else if ((strcasecmp (params[1], "BANTIME")) == 0) {
			watch->bantime	= time_to_sec(params[3]);
		}

		if (!watch->config)
			add_db_entry ("WATCH", (*watch->nickpattern != '\0')? watch->nickpattern : watch->channame, "%d %d %d %s",
				watch->action, watch->options, watch->bantime, watch->message);

		return;
	}

	return;
}

static int evh_chan_join(int argc, void *echan)
{
	evh_channel *echannel = echan;
	Channel			*c;
	struct watch_		*watch;
	struct watchqueue_	*watchque;
	dlink_node		*dl;
	char			mask[512];
	User			*u;

	u	= echannel->u;
	c	= echannel->c;

	if ((MyConnect(u)) || (u->service))
		return EVENT_CONT;

	if ((watch = is_watched (c->name)))
	{
		if (sync_state == BURSTING)
		{
			watchque	= (struct watchqueue_ *) malloc (sizeof(struct watchqueue_));
			dl		= dlink_create ();

			watchque->user	= u;
			watchque->watch	= watch;

			dlink_add_tail (watchque, dl, &watchqueue);

			return EVENT_CONT;
		}

		sendto_logchan ("\002WATCH:\002 %s is joining a watched channel (%s)", u->nick, c->name);

		take_action (u, watch);

		return EVENT_CONT;
	}
	return EVENT_CONT;
}

int evh_user (int argc, void *u)
{
	User *source = u;
	struct watch_		*watch;
	struct watchqueue_	*watchque;
	dlink_node		*dl, *ndl;
	int					status;

	if ((MyConnect(source)) || (source->service))
		return EVENT_CONT;

	DLINK_FOREACH (dl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		if (watch->nickpattern[0] != '\0')
		{
			alog (LOG_DEBUG3, "Watch: evh_user(): Checking %s against watch rule %s", source->nick, watch->nickpattern);

			if (watch->options & OPTION_REGEX)
			{
				if ((status = match_regx (source->nick, watch->nickpattern)) == 0)
				{
					if (sync_state == BURSTING)
					{
						watchque	= (struct watchqueue_ *) malloc (sizeof(struct watchqueue_));
						ndl		= dlink_create ();

						watchque->user	= source;
						watchque->watch	= watch;

						dlink_add_tail (watchque, ndl, &watchqueue);
						return EVENT_CONT;
					}

					take_action (source, watch);
					return EVENT_CONT;
				}
			} else {
          if (match_wild (watch->nickpattern, source->nick, 0))
          {
              if (sync_state == BURSTING)
              {
                watchque        = (struct watchqueue_ *) malloc (sizeof(struct watchqueue_));
                ndl             = dlink_create ();

                watchque->user  = source;
                watchque->watch = watch;

                dlink_add_tail (watchque, ndl, &watchqueue);
                return EVENT_CONT;
              }
              take_action (source, watch);
							return EVENT_CONT;
          }

			}

			alog (LOG_DEBUG3, "Watch: evh_user(): %s did not match any watch rule", source->nick);

		}
	}

	return EVENT_CONT;
}

int evh_watchqueue (int ac, void *nil)
{
	dlink_node		*dl, *tdl;
	struct watch_		*watch;
	struct watchqueue_	*watchque;
	User			*user;
	int			i;

	i = 0;

	if (sync_state == BURSTING)
		return EVENT_CONT;

	DLINK_FOREACH_SAFE (dl, tdl, watchqueue.head)
	{
		if (i > 25)
			return EVENT_CONT;

		i++;

		watchque	= (struct watchqueue_ *) dl->data;
		watch		= watchque->watch;
		user		= watchque->user;

		if (!watch) {
			alog (LOG_DEBUG, "Watch: evh_watchqueue(): Watch rule was removed.");

			dlink_delete	(dl, &watchqueue);
			dlink_free	(dl);
			free		(watchque);
			continue;
		}

		if (watch->nickpattern[0] != '\0')
			sendto_logchan ("\002WATCH:\002 %s joined the network during burst (%s)\n", user->nick, watch->nickpattern);
		else
			sendto_logchan ("\002WATCH:\002 %s joined a watched channel during burst (%s)\n", user->nick, watch->channame);

		take_action (user, watch);


		dlink_delete (dl, &watchqueue);
		dlink_free (dl);
		free (watchque);
	}

	DelTimedEvent ("WatchQueue");
	return EVENT_CONT;
}

struct watch_ *is_watched (char *ch)
{
	dlink_node		*dl;
	struct watch_	*watch;

	if (!ch)
		return 0;

	DLINK_FOREACH (dl, watchlist.head)
	{
		watch	= (struct watch_ *) dl->data;

		if (watch->nickpattern[0] != '\0')
		{
			if ((strcasecmp (watch->nickpattern, ch)) == 0)
				return watch;

			continue;
		}

		if ((strcasecmp (watch->channame, ch)) == 0)
			return watch;
	}

	return NULL;
}

static int take_action	(User *source, struct watch_ *watch)
{
	if (!source || !watch)
		return -1;

	/* Ignore our own bots */
	if (MyConnect(source))
		return;
	if (source->oper && (watch->options & OPTION_IGNOREOPER)) {
		sendto_logchan ("\002WATCH:\002 Not taking action on oper %s for matching watch rule %s", source->nick, (watch->channel)? watch->channame : watch->nickpattern);
		return;
	}
	if ((watch->options & OPTION_IGNOREACCESS) && (check_access(source, ACC_FLAG_USER))) {
		sendto_logchan ("\002WATCH:\002 Not taking action on %s for matching watch rule %s, matched access user", source->nick, (watch->channel)? watch->channame : watch->nickpattern);
		return;
	}

        if (watch->action == ACTION_NONE)
                sendto_logchan ("\002WATCH:\002 no action taken on %s", source->nick);
        else
                sendto_logchan ("\002WATCH:\002 action %s taken on %s, matched watch rule %s", action_num_to_name(watch->action), source->nick,
                                        (watch->channel)? watch->channame : watch->nickpattern);

	switch (watch->action)
	{
		case ACTION_NONE:
			break;
		case ACTION_KICK:
			send_line (":%s MODE %s +b *!*@%s", (IRCd.ts6)? s_Guardian->uid : s_Guardian->nick, watch->channel->name, source->host);
			send_line (":%s KICK %s %s :%s", (IRCd.ts6)? s_Guardian->uid : s_Guardian->nick, watch->channel->name, source->nick, watch->message);
			DelFromChannel (source, watch->channel);
			break;
		case ACTION_KILL:
			ircd_kill (source, watch->message);
			exit_user (source->nick);
			break;
		case ACTION_GLINE:
			ircd_kill (source, watch->message);
			exit_user (source->nick);
			ircd_xline ("G", (IRCd.ts6)? s_Guardian->uid : s_Guardian->nick, source->host, watch->bantime, watch->message);
			break;
		default:
			sendto_logchan ("\002WATCH:\002 Unknown action %d", watch->action);
	}

	return 0;
}

static int find_action	(char *name)
{
	int	i;

	for (i = 0; actions[i].name != NULL; i++)
	{
		if ((strcasecmp (actions[i].name, name)) == 0)
			return actions[i].action;
	}

	return 0;
}

static char *action_num_to_name (int action)
{
	int i;

	for (i = 0; actions[i].name != NULL; i++)
	{
		if (actions[i].action == action)
			return actions[i].name;
	}

	return "Unknown";
}

static void help_watch	(User *source)
{
	sendto_one_help(s_Guardian, source, "WATCH", "Manage the channel/nick watch list");
	return;
}

static void help_watch_ext	(User *source)
{
	sendto_one (s_Guardian, source, "Syntax \002WATCH [OPTION]\002");

	sendto_one (s_Guardian, source, " ");

	sendto_one (s_Guardian, source, "\002WATCH LIST\002");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    Display channels/nicknames we are currently watching.");
	sendto_one (s_Guardian, source, " ");

	sendto_one (s_Guardian, source, "\002WATCH ADD [channel|nickname] [action] [message]");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    Add the given channel or nickname to our watch list.");
    	sendto_one (s_Guardian, source, "    The given channel or nickname is interpreted as a regular expression.");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    Valid actions: none,kick,kickban,kill,gline,kline,gzline,zline");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    \002* NOTES:\002");
	sendto_one (s_Guardian, source, "      - kick is an alias to kickban to prevent loops");
	sendto_one (s_Guardian, source, " ");

	sendto_one (s_Guardian, source, "\002WATCH REMOVE [channel|nickname]\002");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    Remove the given channel or nickname from the watch list.");
	sendto_one (s_Guardian, source, " ");

	sendto_one (s_Guardian, source, "\002WATCH SET [option] [rule] [value]\002");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "    Change settings for a specific watch rule. Rule may be a channel or nickname mask");
	sendto_one (s_Guardian, source, " ");
	sendto_one (s_Guardian, source, "  \002OPTIONS\002");
	sendto_one (s_Guardian, source, "    ACTION");
	sendto_one (s_Guardian, source, "    MESSAGE");
	sendto_one (s_Guardian, source, "    BANTIME");
	sendto_one (s_Guardian, source, "    IGNOREOPERS  - ON/OFF");
	sendto_one (s_Guardian, source, "    IGNOREACCESS - ON/OFF");
	sendto_one (s_Guardian, source, "    REGEX        - ON/OFF");
	sendto_one (s_Guardian, source, " ");

	return;
}


