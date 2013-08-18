/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2010 Omega Dev Team
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
 *    $Id: links.c 2394 2012-06-30 15:56:22Z twitch $
 */

#include "stdinc.h"
#include "server.h"

//server 0 is the uplink server were as
//it will have 2 spots in the link
int servernum = 0;

/******************************************************/

Link	*new_serv (Link *uplink, char *servername)
{
	Link		*tmp;
	dlink_node	*dl;

	if ((tmp = find_serv (servername)))
		return tmp;

	tmp	= (Link*) malloc(sizeof(Link));
	dl	= dlink_create ();

	memcounts.servsize	+= sizeof(tmp);
	memcounts.servcnt++;

	if (!valid_servername(servername))
		Log("Invalid servername from %s",servername);

	strlcpy (tmp->name, servername, sizeof(tmp->name));

	tmp->linktime	= time(NULL);
	tmp->eos		= 0;
	tmp->uplink		= (uplink)? uplink : NULL;
	tmp->sid[0]		= '\0';

	dlink_add_tail (tmp, dl, &servlist);

	servernum++;

	sendto_logchan ("\002Server Connect:\002 %s connecting to %s", servername, (uplink)? uplink->name : "<No Uplink>");
    Event ("SERVERCONN", 1, tmp);

	return tmp;
}

/*****************************************/

Link* find_serv(char *serv)
{
    Link        *li;
    dlink_node  *dl;

    if (!serv)
        return NULL;

    DLINK_FOREACH (dl, servlist.head)
    {
        li  = dl->data;

        if (!li->name)
            continue;

        if ((strcasecmp (li->name, serv)) == 0)
            return li;

        if (li->sid[0] != '\0')
            if ((strcasecmp (li->sid, serv)) == 0)
                return li;
    }

	return NULL;
}

/*****************************************/
void exit_serv (char *serv)
{
    Link        *li, *s;
    User        *u;
    dlink_node  *dl, *tdl;

    if (!serv)
        return;

    if (!(s = find_serv (serv)))
        return;

    DLINK_FOREACH_SAFE (dl, tdl, servlist.head)
    {
        li  = dl->data;

        if (li->uplink)
        {
            if ((strcasecmp (li->uplink->name, s->name)) == 0)
                exit_serv (li->name);
        }
    }

    sendto_logchan ("\002Netsplit:\002 %s split from %s", s->name, s->uplink->name);
    Event ("SERVEREXIT", 1, s);

    /* Exit users that are on the server we are exiting */

    DLINK_FOREACH_SAFE (dl, tdl, userlist.head)
    {
        u   = dl->data;

        if ((u) && (u->serv))
            if ((strcasecmp(s->name, u->serv->name)) == 0)
                exit_one_user(u, NULL);
    }

    dl = dlink_find_delete (s, &servlist);
    dlink_free (dl);

	memcounts.servsize	-= sizeof(s);
	memcounts.servcnt--;
    free (s);

    return;

}

void exit_all_servs (void)
{
	Link		*li;
	dlink_node	*dl, *tdl;

	DLINK_FOREACH_SAFE (dl, tdl, servlist.head)
	{
		li	= dl->data;

		if ((strcasecmp (li->name, CfgSettings.servername)) == 0)
		{
			li->uplink	= NULL;
			continue;
		}
		dlink_delete (dl, &servlist);
		dlink_free (dl);
		free (li);
	}

	return;
}

/***************************************/

void  update_eos(char *serv)
{
	Link* tmp = find_serv(serv);

	if (!tmp)
		return;
	else
		tmp->eos = 1;

	return;
}

/*******************************************/

int link_count(void)
{
	return servernum;
}

/*******************************************/

int burst_local_servers (int argc, void *args) {
    char **argv = args;
    Link        *serv;
    dlink_node  *dl;

    DLINK_FOREACH (dl, servlist.head) {
        serv    = (Link *) dl->data;

        if (serv->uplink) {
            if (strcasecmp (serv->uplink->name, CfgSettings.servername) == 0)
                ircd_server (serv);
        }
    }
}
