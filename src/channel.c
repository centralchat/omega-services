/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
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
 *    $Id: channel.c 2252 2012-01-22 09:21:23Z twitch $
 */

#include "stdinc.h"
#include "server.h"

extern Channel* chan_list;

struct CMode ircd_cmodes[128] = {
    { "q",    1,    1,      "~"  },  // Channel owner
    { "a",    1,    1,      "&"  },  // Protected
    { "o",    1,    1,      "@"  },  // Channel operator
    { "h",    1,    1,      "%"  },  // Channel halfop
    { "v",    1,    1,      "+"  },  // Channel voice

    { "n",    0,    0,      0    },  // No external messages
    { "t",    0,    0,      0    },  // Only ops may set the topic
    { "r",    0,    0,      0    },  // Registered channel
    { "p",    0,    0,      0    },  // Private channel
    { "k",    1,    0,      0    },  // Channel has a key
    { "l",    1,    0,      0    },  // Channel has a limit
	{ "f", 	  1,	0,		0	 },  //Channel has flood mode
    { NULL,   0,    0,      NULL }
};

/**
 * Find a mode string by its retrospective symbol
 *
 *  @param symbol Symbol of the mode we are searching for.
 *
 *  @return return the location of the mode within our supported
 *          mode structure or -1 if the mode isn't supported or
 *          we failed for whatever reason.
 */


int findcmode_bysymbol (char *symbol)
{
    int     i;

    if (!symbol)
        return -1;

    for (i = 0; i < 128; i++)
    {
        if (!ircd_cmodes[i].symbol)
            continue;

        if ((strcmp(ircd_cmodes[i].symbol, symbol)) == 0)
            return i;
    }

    return -1;
}

/************************************************/
/**
 * Allocate a new channel structure as well as initialize
 * the defaults for that channel.
 *
 * @param chan the name of the channel we are creating a structure for
 * @return Channel* Return a pointer to the channel structure regardless.
 *         if the channel already exists return a pointer to that as well.
 */


Channel* new_chan (char *chan)
{
    Channel     *tmp;
    dlink_node  *dl = dlink_create ();

    tmp = find_channel(chan);

    if (tmp)
		return tmp;

    tmp = (Channel *)malloc(sizeof(Channel));
    strlcpy (tmp->name, chan, sizeof(tmp->name));

	memcounts.chansize	+= sizeof(tmp);
	memcounts.channels++;

    dlink_add_tail (tmp, dl, &channels);

    tmp->onlyoptopic    = 1;
    tmp->external       = 1;
    tmp->haskey         = 0;
    tmp->limit          = 0;
    tmp->usercnt        = 0;

    tmp->registered     = 0;
    tmp->flood          = 0;
    tmp->regonly        = 0;
    tmp->linked         = 0;
    tmp->operonly       = 0;
    tmp->priv           = 0;
    tmp->ssl            = 0;
    tmp->invite         = 0;
    tmp->audit          = 0;

    tmp->channelts      = time(NULL);

    tmp->users.head = tmp->users.tail = NULL;

    strlcpy (tmp->unknownmodes, "", sizeof(tmp->unknownmodes));

	alog(8,"New channel created %s",tmp->name);

    return tmp;
}

/**************************************************/
/**
 * Find a channel structure within the list by channel
 * name.
 *
 * @param chan Channel name
 *
 * @return Channel* Return the structure we found, or
 *         NULL if it cant be found or we fail for w/e
 *         reason.
 */


Channel* find_channel(char *chan)
{
    Channel     *c;
    dlink_node  *dl;

    if (!chan)
        return NULL;

    DLINK_FOREACH (dl, channels.head)
    {
        c   = dl->data;

        if ((strcasecmp (c->name, chan)) == 0)
            return c;
    }

    return NULL;

}

/***************************************************/

/**
 * Check if a user is in a channel.
 *
 * @param user User we are checking for.
 * @param ch Channel we are checking against.
 *
 * @return True if the user is within channel, or
 *         false if user is not.
 */

int in_channel (User *user, Channel *ch)
{
	dlink_node	*dl;
	Channel		*c;

    if (!user || !ch)
        return 0;

	DLINK_FOREACH (dl, user->channels.head)
	{
		c	= (Channel *) dl->data;

		if ((strcasecmp (c->name, ch->name)) == 0)
			return 1;
	}

	return 0;
}


/***************************************************/

void AddToChannel (char *user, Channel *c)
{
    ChanUser        *cu;
    User            *u;
    dlink_node      *dl;

    int             cmode, op, halfop, protect, owner, voice;

    char            tmp[512];
	evh_channel		evhinfo;

    if (!user || !c)
        return;

    while ((*user == '~') || (*user == '&') || (*user == '@') ||
            (*user == '%') || (*user == '+') || (*user == '*'))
    {
        snprintf (tmp, sizeof(tmp), "%c", *user);

        cmode   = findcmode_bysymbol (tmp);

        user++;

	 if (*user == ',')
		user++;

        switch (cmode)
        {
            case 2:
                op  = 1;
                break;
        }

        memset(tmp,0,sizeof(tmp));
    }

    if (!(u = find_user (user)))
        return;

    if (in_channel (u, c))
        return;

    if (!(cu = new_chanuser (u)))
        return;

    dl = dlink_create ();

    if (owner)
        cu->owner   = 1;
    if (protect)
        cu->protect = 1;
    if (op)
        cu->op      = 1;
    if (halfop)
        cu->halfop  = 1;
    if (voice)
        cu->voice   = 1;

    dlink_add_tail (cu, dl, &c->users);
    c->usercnt++;

    dl = dlink_create ();

    dlink_add_tail (c, dl, &u->channels);

    evhinfo.c   = c;
    evhinfo.u   = u;
	Event ("CHANJOIN", 1, &evhinfo);

    return;
}

void AddToChannelU (User *user, Channel *c)
{
    ChanUser        *cu;
    dlink_node      *dl;

    int             cmode, op, halfop, protect, owner, voice;

    char            tmp[512];
	evh_channel		evhinfo;

    if (!user || !c)
        return;

    if (in_channel (user, c))
        return;

    if (!(cu = new_chanuser (user)))
        return;

    dl = dlink_create ();

    if (owner)
        cu->owner   = 1;
    if (protect)
        cu->protect = 1;
    if (op)
        cu->op      = 1;
    if (halfop)
        cu->halfop  = 1;
    if (voice)
        cu->voice   = 1;

    dlink_add_tail (cu, dl, &c->users);
    c->usercnt++;

    dl = dlink_create ();

    dlink_add_tail (c, dl, &user->channels);

    if (MyConnect(user) && (sync_state != BURSTING))
        ircd_join (user, c);

	evhinfo.c	= c;
	evhinfo.u	= user;
    
	Event ("CHANJOIN", 1, &evhinfo);
}

void KickFromChannel (User *u, Channel *c, char *message) {
    ChanUser    *cu;
    dlink_node  *dl;
    evh_channel evhinfo;

    if (!(cu = find_chanuser (u, c)))
        return;

    evhinfo.c   = c;
    evhinfo.u   = u;

    strlcpy (evhinfo.message, message, sizeof(evhinfo.message));

    dl  = dlink_find_delete (cu, &c->users);
    dlink_free  (dl);
    free        (cu);

    dl  = dlink_find_delete (c, &u->channels);
    dlink_free  (dl);

    Event ("CHANKICK", 1, &evhinfo);

    if (!c->usercnt) {
        dl  = dlink_find_delete (c, &channels);
        dlink_free  (dl);

        alog (LOG_DEBUG2, "Freeing c at %p", c);
        free        (c);
    }
}

void DelFromChannel (User *user, Channel *c)
{
    ChanUser    *cu;
    dlink_node  *dl;
	evh_channel	evhinfo;

    if (!(cu = find_chanuser (user, c)))
        return;

	evhinfo.c	= c;
	evhinfo.u	= user;

	Event ("CHANPART", 1, &evhinfo);

/*
    if (MyConnect (user))
        ircd_part (user, c, "Leaving");
*/

    dl = dlink_find_delete (cu, &c->users);
    dlink_free (dl);
	memcounts.chanusize -= sizeof(cu);
	memcounts.chanucnt--;
    free (cu);
    c->usercnt--;

    dl = dlink_find_delete (c, &user->channels);
    dlink_free (dl);

	if (c->usercnt <= 0)
	{
		memcounts.chansize	-= sizeof(c);
		memcounts.channels--;
	
		dl = dlink_find_delete (c, &channels);
		dlink_free (dl);
		free (c);
	}
}


struct ChanUser *new_chanuser (User *user)
{
    struct ChanUser     *chanuser;

    chanuser    = (struct ChanUser *) malloc (sizeof(struct ChanUser));

	memcounts.chanusize	+= sizeof(chanuser);
	memcounts.chanucnt++;

    chanuser->user  = user;

    chanuser->owner     = 0;
    chanuser->protect   = 0;
    chanuser->op        = 0;
    chanuser->halfop    = 0;
    chanuser->voice     = 0;

    return chanuser;
}

struct ChanUser *find_chanuser (User *u, Channel *c)
{
    struct ChanUser     *cu;
    dlink_node          *dl;

    DLINK_FOREACH (dl, c->users.head)
    {
        cu  = dl->data;

        if (!cu->user)
            continue;

        if ((strcasecmp (cu->user->nick, u->nick)) == 0)
            return cu;
    }

    return NULL;
}

char * create_sjoin_nicklist (Channel *c)
{
	static char		nicklist[1024];
	struct ChanUser	*cu;
	dlink_node		*dl;

	nicklist[0]		= '\0';

	DLINK_FOREACH (dl, c->users.head)
	{
		cu	= dl->data;

		if (!cu->user)
			continue;

		if (nicklist[0] != '\0')
			strlcat (nicklist, " ", sizeof(nicklist));

		if (cu->op)
			strlcat (nicklist, ircd_cmodes[2].symbol, sizeof(nicklist));

		strlcat (nicklist, cu->user->nick, sizeof(nicklist));
	}

	return nicklist;
}


char * create_chanmode_string (Channel *c)
{
    static char     chanmode_string[1024];
    char            params[1024];
    char            limit[10];

    chanmode_string[0]  = '\0';
    params[0]           = '\0';

    strlcat (chanmode_string, "+", sizeof(chanmode_string));

    if ((c->registered) && (ircd_cmodes[7].character != NULL))
        strlcat (chanmode_string, ircd_cmodes[7].character, sizeof(chanmode_string));
    if ((c->onlyoptopic) && (ircd_cmodes[6].character != NULL))
        strlcat (chanmode_string, ircd_cmodes[6].character, sizeof(chanmode_string));
    if ((c->external) && (ircd_cmodes[5].character != NULL))
        strlcat (chanmode_string, ircd_cmodes[5].character, sizeof(chanmode_string));

	strlcat (chanmode_string, c->unknownmodes, sizeof(chanmode_string));

    if ((c->haskey) && (ircd_cmodes[9].character != NULL))
    {
        strlcat (chanmode_string, ircd_cmodes[9].character, sizeof(chanmode_string));

        if (params[0] == '\0')
            strlcpy (params, c->key, sizeof(params));
        else
        {
            strlcat (params, " ", sizeof(params));
            strlcat (params, c->key, sizeof(params));
        }
    }

    if ((c->limit > 0) && (ircd_cmodes[10].character != NULL))
    {
        strlcat (chanmode_string, ircd_cmodes[10].character, sizeof(chanmode_string));

        sprintf (limit, "%d", c->limit);

        if (params[0] == '\0')
            strlcpy (params, limit, sizeof(params));
        else
        {
            strlcat (params, " ", sizeof(params));
            strlcat (params, limit, sizeof(params));
        }
    }

    if (params[0] != '\0')
    {
        strlcat (chanmode_string, " ", sizeof(chanmode_string));
        strlcat (chanmode_string, params, sizeof(chanmode_string));
    }

    return chanmode_string;
}

void read_chanmode_string (Channel *c, char *modes, char **params)
{
    int     i, toadd, found, paramcnt;
    char    mode[5];

    char    *p = NULL;

    s_assert (c != NULL);

    paramcnt    = 0;

    while (*modes != '\0')
    {
		sprintf (mode, "%c", *modes);
        switch (*modes)
        {
            case '+':
                toadd   = 1;
                break;
            case '-':
                toadd   = 0;
                break;
	     case ',':
	         break;
            default:
                found       = 0;
                for (i = 0; i < 128; i++)
                {
                    if (!ircd_cmodes[i].character)
                        continue;
                    s_assert (ircd_cmodes[i].character != NULL);
                    /**
                     ** Work our the nasty SJOIN params (mainly keys and limits)
                     */

                    if ((strcmp (ircd_cmodes[i].character, mode)) == 0)
                    {
                        found   = 1;
                        if ((ircd_cmodes[i].setparam != 0) && (toadd))
                        {
                            if (params[paramcnt] == NULL)
                                continue;

                            p   = params[paramcnt];
                            paramcnt++;
                        }
                        if ((ircd_cmodes[i].unsetparam != 0) && (!toadd))
                        {
                            if (params[paramcnt] == NULL)
                                continue;

                            p = params[paramcnt];
                            paramcnt++;
                        }
                        switch (i)
                        {
                            case 5:
                                c->external     = (toadd)? 1 : 0;
                                break;
                            case 6:
                                c->onlyoptopic  = (toadd)? 1 : 0;
                                break;
                            case 9: // Channel key
                                c->haskey       = (toadd)? 1 : 0;
                                if (toadd)
                                    strlcpy (c->key, p, sizeof(c->key));
                                else
                                    strlcpy (c->key, "", sizeof(c->key));
                                break;
							case 10: // Channel limit
                                c->limit    = (toadd)? atol(p) : 0;
                                break;
							case 11: // Mode f
								if (toadd)
									strlcpy(c->modef, p, sizeof(c->key));
								else
									strlcpy(c->modef, "", sizeof(c->key));
								break;
                        }
                        break;
                    }
					if (found)
							break;
                }
				if ((!found) && (*mode != 'b'))
						strlcat (c->unknownmodes, mode, sizeof(c->unknownmodes));
            break;

        }
        modes++;
    }

    return;
}

/* vim: set tabstop=4 shiftwidth=4 expandtab */

