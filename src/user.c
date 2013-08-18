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
*    $Id: user.c 2394 2012-06-30 15:56:22Z twitch $
*/

/**********************************/

#include "stdinc.h"
#include "server.h"

int usercnt;
int maxusers;

char *ircd_umodes[128] = {
	NULL
};


/**********************************/

User* AddUser(char *nick, char *user, char *host, char *realname, char *server, char *modes, char *uid, char *vhost, char *ip)
{
	User    *u;
	Link    *li;

	if ((u = new_user (nick)) == NULL)
		return NULL;

	s_assert(u != NULL);

	if ((host != NULL) && (valid_hostname(host)))
		strlcpy (u->host, host, sizeof(u->host));
	else
		snprintf(u->host,sizeof(u->host),"invalid.hostname");


	strlcpy (u->realname, realname, sizeof(u->realname));

	if (user)
		strlcpy (u->user, user, sizeof(u->user));

	if (modes)
		read_usermode_string(u, modes);

	//make sure we have ts6 on (This is just a fail safe)
	if ((IRCd.p10) || (IRCd.ts6 && uid))
		strlcpy (u->uid, uid, sizeof(u->uid));

	if (!(li = find_serv (server)))
	{
		sendto_logchan ("\002Error:\002 %s (%s) attempting to connect from non existent server (%s)", nick, host, server);
		exit_user (nick);
		return NULL;
	}

	if (li)
		u->serv = li;

	if ((vhost != NULL) && (valid_hostname(vhost)))
		strlcpy(u->virthost,vhost,sizeof(u->virthost));

	if (ip)
        	strlcpy (u->ip, ip, sizeof(u->ip));

	usercnt++;

	if (!maxusers)
		maxusers = usercnt;

    setAccess (u);
	Event ("USERCONN", 1, u);

	return u;

}

/**********************************/

User *new_user (char *nick)
{
	User*  tmp;
	dlink_node  *dl = dlink_create();
	int i;

	tmp = find_user(nick);

	if (tmp)
	{
		fprintf (stderr, "We already know about user: %s\n", nick);
		return NULL;
	}

	tmp = (User*) malloc(sizeof(User));

	memcounts.userssize	+= sizeof(User);
	memcounts.users++;

	strlcpy (tmp->nick, nick, sizeof(tmp->nick));

	tmp->uid[0]	    = '\0';
	tmp->version[0]	= '\0';
    tmp->ip[0]      = '\0';

    tmp->age        = time(NULL);

	tmp->oper       = 0;
	tmp->ssl        = 0;
	tmp->bot        = 0;
	tmp->service    = 0;
	tmp->webtv      = 0;
	tmp->cloak      = 0;
	tmp->vhost      = 0;
	tmp->invis      = 0;
	tmp->priv       = 0;
    tmp->myconnect  = 0;
    tmp->registered = 0;
    tmp->admin      = 0;
    tmp->hideoper   = 0;
	tmp->fantasy	= 0;
    tmp->lines      = 0;
    tmp->ignore     = 0;

	tmp->lastsent   = time(NULL);

	memset(tmp->access,0,sizeof(tmp->access));

	tmp->channels.head = tmp->channels.tail = NULL;
	tmp->metadata.head = tmp->metadata.tail = NULL;
	
	dlink_add_tail (tmp, dl, &userlist);

	return tmp;
}

/**********************************/

/**
** find_user
**
*/

User* find_user(char *user)
{
	User        *u;
	dlink_node  *dl;

    if (!user)
        return NULL;

    alog (LOG_DEBUG3, "find_user(): searching for %s", user);
	DLINK_FOREACH (dl, userlist.head)
	{
		u   = dl->data;
		
		if (!u->nick) 
			continue;
			
		if ((strcasecmp (u->nick, user)) == 0)
		    return u;

		if (HasId(u)) {
            alog (LOG_DEBUG3, "find_user(): checking uid %s", u->uid);
		    if ((strcasecmp (u->uid, user)) == 0)
		        return u;
        }
	}

	return NULL;

}

User* find_uid (char *uid)
{
	User        *u;
	dlink_node  *dl;

    alog (LOG_DEBUG3, "find_uid(): search for %s", uid);

	DLINK_FOREACH (dl, userlist.head)
	{
		u   = dl->data;

		if (HasId(u)) {
		    if ((strcasecmp (u->uid, uid)) == 0)
		        return u;
        }
        else
            alog (LOG_DEBUG3, "find_uid(): entry for %s has no uid!", u->nick);
	}

	return NULL;
}

/************************************************/
/**
* exit_user() - Removes a user from the list
* @param user - The nick of the user to remove
* @return void
*/


void exit_user (char *user)
{
	User        *u;
	dlink_node  *dl, *tdl;
	Channel     *c;
	Metadata    *md;
	u   = find_user (user);

	if (u)
	{
       /*
		if (logclients)
		{
			sendto_logchan("\002Quit:\002 %s (%s) exiting from %s",
			u->nick,u->host, (u->serv)? u->serv->name : "Unknown");
		}
		*/
		

		DLINK_FOREACH_SAFE (dl, tdl, u->channels.head)
		{
			c   = dl->data;
			DelFromChannel (u, c);
		}
                
        DLINK_FOREACH_SAFE(dl, tdl, u->metadata.head) {
		md = dl->data;

		dlink_delete(dl, &u->metadata);
		dlink_free(dl);
		free(md);

	}
		if (MyConnect (u))
			send_line (":%s QUIT :Exiting", (IRCd.ts6)? u->uid : u->nick);
		
		Event ("USEREXIT", 1, u);
		alog(LOG_USER,"QUIT: %s (%s) exiting from %s",u->nick,u->host, (u->serv)? u->serv->name : "Unknown");


		dl  = dlink_find_delete (u, &userlist);
		dlink_free (dl);

		memcounts.users--;
		memcounts.userssize -= sizeof(User);

		if (u)
			free (u);

		usercnt--;
		return;
	}
}

void exit_one_user (User *u, char *message)
{
	dlink_node		*dl, *tdl;
	Channel			*c;
        Metadata		*md;
	if (!u)
	return;

	DLINK_FOREACH_SAFE (dl, tdl, u->channels.head) {
		c	= dl->data;
		DelFromChannel (u, c);
	}
	
    DLINK_FOREACH_SAFE(dl, tdl, u->metadata.head) {
		md = dl->data;

		dlink_delete(dl, &u->metadata);
		dlink_free(dl);
		free(md);

	}
	
	if (MyConnect (u))
		send_line (":%s QUIT :%s", (IRCd.ts6)? u->uid : u->nick, (message)? message : "Exiting");

	Event ("USEREXIT", 1, u);

	alog(LOG_USER,"QUIT: %s (%s) exiting from %s",u->nick,u->host, (u->serv)? u->serv->name : "Unknown");

	dl	= dlink_find_delete (u, &userlist);
	dlink_free (dl);

	memcounts.users--;
	memcounts.userssize  -= sizeof(u);
        
	if (u)
	free (u);
	return;
}

void exit_local_users (char *message)
{
	User		*u;
	Channel		*c;
	dlink_node	*dl, *tdl, *cdl, *ctdl;

	DLINK_FOREACH_SAFE (dl, tdl, userlist.head)
	{
		u	= dl->data;

		if (MyConnect (u))
		exit_one_user (u, message);
	}

	return;
}

void exit_remote_users (void)
{
	User		*u;
	Channel		*c;
	dlink_node	*dl, *tdl;

	DLINK_FOREACH_SAFE (dl, tdl, userlist.head)
	{
		u	= dl->data;
		if (!MyConnect(u))
			exit_one_user (u, NULL);
	}

	return;
}

int burst_local_users (int argc, void *args)
{
	char 		**argv = (char**)args;
	User		*user;
	Channel		*chan;
	dlink_node	*dl, *cdl;

	DLINK_FOREACH (dl, userlist.head)
	{
		user	= dl->data;
		if (!MyConnect (user)) // Theoretically if we are bursting we shouldn't have any non-local clients
			return EVENT_STOP;

		//ircd_add_user (user);
		DLINK_FOREACH (cdl, user->channels.head)
		{
			chan	= cdl->data;
			ircd_join(user, chan);
		}
	}
	return EVENT_OK;
}

/************************************************/
/**
* introduce_users() - Configures our client structures
* @return void;
*/

int introduce_users (int ac, void *nil)
{
    User        *user;
    dlink_node  *dl;

    DLINK_FOREACH (dl, userlist.head)
    {
        user    = dl->data;

        if (!MyConnect (user)) // Theoretically if we are bursting we shouldn't have any non-local clients
            return EVENT_STOP;

        ircd_add_user (user);
    }
    return  EVENT_OK;
}


/**************************************************/

void NewNick(User *u, char *nick)
{
	User *user;

	evh_nick enick;
	
	if (u && nick)
	{

		if (!(user = find_user(nick)))
		{
			strlcpy (enick.oldnick, u->nick, sizeof(enick.oldnick));
			alog(LOG_USER,"NICKCHANGE: %s has changed his or her nickname to %s",u->nick,nick);
			memset(u->nick,'\0', sizeof(u->nick));
			strlcpy(u->nick,nick,sizeof(u->nick));

			enick.u		= u;

			Event ("USERNICK", 1, &enick);
			return;
		}
		alog(5,"Unable to change nickname %s to %s in structure [NICK EXISTS]",u->nick,nick);
		return;
	}

	return;
}

/*********************************************/
/*
static char *ircd_umodes[128] = {
	"r",	// Registered Nickname 0
	"o",	// IRC Operator 1
	"i",	// Invisible 2
	"w",	// Wallops 3
	NULL,	// Using a secure connection 4
	NULL,	// Is a bot 5
	NULL,	// Is a network service 6
	NULL,	// Has a cloaked host 7
	NULL,	// Has a virtual host 8
	NULL,	// Private 9
	"a",	// Server Administrator 10
	NULL,	// WebTV
	NULL
};
*/


char * create_usermode_string (User *u)
{
	static char	usermode_string[1024];

	usermode_string[0] = '\0';
	strlcat (usermode_string, "+", sizeof(usermode_string));

	if ((u->registered) && (ircd_umodes[0] != NULL))
	strlcat (usermode_string, ircd_umodes[0], sizeof(usermode_string));
	if ((u->oper) && (ircd_umodes[1] != NULL))
	strlcat (usermode_string, ircd_umodes[1], sizeof(usermode_string));
	if ((u->invis) && (ircd_umodes[2] != NULL))
	strlcat (usermode_string, ircd_umodes[2], sizeof(usermode_string));
	if ((u->ssl) && (ircd_umodes[4] != NULL))
	strlcat (usermode_string, ircd_umodes[4], sizeof(usermode_string));
	if ((u->bot) && (ircd_umodes[5] != NULL))
	strlcat (usermode_string, ircd_umodes[5], sizeof(usermode_string));
	if ((u->service) && (ircd_umodes[6] != NULL))
	strlcat (usermode_string, ircd_umodes[6], sizeof(usermode_string));
	if ((u->cloak) && (ircd_umodes[7] != NULL))
	strlcat (usermode_string, ircd_umodes[7], sizeof(usermode_string));
	if ((u->vhost) && (ircd_umodes[8] != NULL))
	strlcat (usermode_string, ircd_umodes[8], sizeof(usermode_string));
	if ((u->priv) && (ircd_umodes[9] != NULL))
	strlcat (usermode_string, ircd_umodes[9], sizeof(usermode_string));
	if ((u->admin) && (ircd_umodes[10] != NULL))
	strlcat (usermode_string, ircd_umodes[10], sizeof(usermode_string));
    if ((u->hideoper) && (ircd_umodes[12] != NULL))
    strlcat (usermode_string, ircd_umodes[12], sizeof(usermode_string));


	return usermode_string;
}

void read_usermode_string (User *u, char *umodes)
{
	int i, toadd, found;

	char pattern[125];

	char *ausers;
	char mode[2];

	Access* a;
	dlink_node *dl;

	toadd = 0;
	s_assert (u != NULL);
        
       // Event("USERMODE", 1, u);

	while (*umodes != '\0')
	{
		switch (*umodes)
		{
		case '+':
			toadd = 1;
			break;
		case '-':
			toadd = 0;
			break;
		default:
			found = 0;

			memset(mode,0,sizeof(mode));
			snprintf(mode,sizeof(mode),"%c",*umodes);

			alog(LOG_DEBUG2, "Looking for mode %s", mode);

			for (i = 0; i < 128; i++)
			{
				if (ircd_umodes[i] == NULL) {
					alog(LOG_DEBUG2, "No umode associated with index %d", i);
					continue;
				}

				alog (LOG_DEBUG2, "comparing %s against ircd_umodes[%d]: %s", mode, i, ircd_umodes[i]);

				if (strcmp(ircd_umodes[i], mode) == 0)
				{
					alog(LOG_DEBUG2, "Found mode %s index: %d", mode, i);
					found = 1;

					switch (i)
					{
					case 0:
						// Registered Nickname
						u->registered	= (toadd)? 1 : 0;
						break;
					case 1: // IRC Operator

						alog (LOG_DEBUG2, "Applying IRC Operator status to %s", u->nick);

						if (toadd)
						sendto_logchan ("\002Oper:\002 %s is now an IRC Operator.", u->nick);
						else
						sendto_logchan ("\002Oper:\002 %s is no longer an IRC Operator.\n", u->nick);

						u->oper	= (toadd)? 1 : 0;
						setAccess (u);

						break;
					case 2: // Invisible
						u->invis	= (toadd)? 1 : 0;
						break;
					case 3: // Wallops
						break;
					case 4: // Using a secure connection
						u->ssl	 	= (toadd)? 1 : 0;
						break;
					case 5: // Is a bot
						u->bot		= (toadd)? 1 : 0;
						break;
					case 6: // Is a network service
						u->service	= (toadd)? 1 : 0;
						break;
					case 7: // Using a cloaked host
						u->cloak	= (toadd)? 1 : 0;
						break;
					case 8: // Using a virtual host
						u->vhost	= (toadd)? 1 : 0;
						break;
					case 9: // Private
						u->priv	= (toadd)? 1 : 0;
						break;
					case 10: // Server Administrator
						if (toadd)
						sendto_logchan ("\002Oper:\002 %s is now a Server Administrator.", u->nick);
						else
						sendto_logchan ("\002Oper:\002 %s is no longer a Server Administrator.\n", u->nick);
						u->admin	= (toadd)? 1 : 0;
						break;
					case 11: // WebTV
						u->webtv	= (toadd)? 1 : 0;
						break;
                    case 12:
                        u->hideoper = (toadd)? 1 : 0;
                        break;
					}

					break;
				}

			}


			break;
		}

		umodes++;
	}
          
	return;
}

/*************************************************/
/***
* Initialize a client structure for use as one of our clients
* This will setup all the necessary information and return a pointer
* to the structure
*/


User* NewClient(char *nick,char* ident,char* host,char* name)
{
	User    *u;
	Link    *li;

	if (!(u = new_user (nick)))
	{
		fprintf (stderr, "Unable to create user\n");
		return NULL;
	}

	if (!(li = find_serv (CfgSettings.servername)))
	{
		fprintf (stderr, "Unable to find our server entry: %s\n", CfgSettings.servername);
		return NULL;
	}

	strlcpy (u->host, host, sizeof(u->host));
	strlcpy (u->realname, name, sizeof(u->realname));
	strlcpy (u->user, ident, sizeof(u->user));
	strlcpy (u->ip, "127.0.0.1", sizeof(u->ip));


	if (IRCd.ts6)
		strlcpy (u->uid, (char *)generate_uid(), sizeof(u->uid));
	if (IRCd.p10)
		generate_p10_uid (u);

	read_usermode_string(u, CfgSettings.umode);

	u->age      	= time(NULL);     //Set TS to 0 so we win in collisions :)
	u->oper     	= 1;
	u->service  	= 1;
	u->myconnect	= 1;
	u->fantasy 		= 1;

	u->ignore       = 0;
	u->lastsent     = time(NULL);

	u->serv         = li;

	if (sync_state != BURSTING)
		ircd_add_user (u);

	return u;
}

int DelClient(char *client)
{
	User *u;
	
	if (!(u = find_user(client)))
			return -1;
	else
		exit_one_user(u,"Client destroyed by server.");
		
	return 0;
}

/******************************************/
/**
* isOurClient() - Determines whether or not
*                 a client is on our server
*    @param client - The client to check for
*    @return User*
*                  NULL  - Fail
*                  User* - Success
*/


User* isOurClient(char *client)
{
	User* u;

	if (!(u = find_user(client)))
		return NULL;

	if (strcasecmp(u->serv->name,CfgSettings.servername) == 0)
		return u;

	return NULL;
}



/*******************************************/






