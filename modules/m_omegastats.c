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
 *    $Id: m_omegastats.c 2394 2012-06-30 15:56:22Z twitch $
 */

#include "stdinc.h"
#include "server.h"

#define NAME	"m_omegastats"
#define	AUTHOR	"Jer"
#define VERSION	"$Revision: 1393 $"

#define HTTP_ADDR	"omega-services.org"
#define HTTP_SERV	"omega-services.org"
#define HTTP_PORT	80

Socket	*statsock;

int      omegastats_load ();
void     omegastats_close();

static void		sock_read	(Socket *s, int e);
static void		sock_write	(void *);
static void     omegastats_closesock ();

int	evh_omegastats	(int, void*);
int	evh_first_run	(int, void*);

static int stats_sent   = 0;

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,5),omegastats_load, omegastats_close);

int omegastats_load ()
{
	AddTimedEvent	("Omega Stats First Run", evh_first_run, 30);
	return MOD_CONT;
}

void omegastats_close ()
{
	dlink_node	*dl;
	DelTimedEvent	("Omega Statistical Post");

	if ((statsock = find_socket ("statsock")))
	{
		close (statsock->sd);
		dl	= dlink_find_delete (statsock, &sockets);
		dlink_free (dl);
		free (statsock);
	}

	return;
}

static void omegastats_closesock ()
{
    dlink_node  *dl;

    if ((statsock = find_socket ("statsock")))
       	delete_socket(statsock);
    return;
}

int evh_first_run(int ac, void *unused)
{
	evh_omegastats (0, NULL);
	DelTimedEvent("Omega Stats First Run");
	AddTimedEvent	("Omega Statistical Post", evh_omegastats, 900);
}

static void	sock_read	(Socket *s, int e)
{
	char	*msg;

	while (s->fullLines > 0)
	{
		msg	= GetLine (s);

		if (msg)
			free (msg);
	}

    if (stats_sent)
        omegastats_closesock ();

	return;
}

static void sock_write	(void *args)
{
	int		ret;
	char	content[2048];
	char	headers[2048];

	Socket *s = args;
	
	//printf("Calling omega services sock write\n");
	
	memset (content, '\0', sizeof(content));
	memset (headers, '\0', sizeof(headers));

	snprintf (content, 2048, "servername=%s&uplink=%s&version=%s&protocol=%s&uplinkname=%s&start=%d", CfgSettings.servername, CfgSettings.uplink,
				VERSION_STRING_DOTTED, CfgSettings.protocol, (Uplink)? Uplink->name : "not%20connected", (int) starttime);
	snprintf (headers, 2048, "User-Agent: Omega/%s m_omegastats.c/%s\nHost: %s\nContent-Length: %d\nContent-Type: application/x-www-form-urlencoded", VERSION_STRING_DOTTED, VERSION, HTTP_SERV, (int)strlen(content));

	//printf ("Sending:\nPOST /omega/omega.php HTTP/1.1\n%s\n\n%s\r\n\r\n", headers, content);

	#if 0
		ret = sendto_socket (s, "PUT /stats/api HTTP/1.1\n%s\n\n%s\r\n\r\n", headers, content);
        #endif 	
	ret = sendto_socket (s, "POST /stats/omega.php HTTP/1.1\n%s\n\n%s\r\n\r\n", headers, content);

	if (ret == 0)
	{
		s->flags	&= ~SOCK_CONNECTING;
		s->flags	&= ~SOCK_WRITE;
		s->flags	|= SOCK_READ;

        stats_sent  = 1;
		return;
	}

	return;
}

int evh_omegastats	(int argc, void* unused)
{
	int					ret;
	struct sockaddr_in	sa;
	dlink_node			*dl;
	char			vhost[100];
	unsigned char		addr[16];

	if (!(statsock = find_socket ("statsock")))
	{
       		stats_sent  = 0;

		statsock	= add_new_socket ((void *) sock_read, (void*)sock_write);
		statsock->flags	|= SOCK_CONNECTING|SOCK_WRITE;

		strlcpy (statsock->name, "statsock", sizeof(statsock->name));

		inet_pton(AF_INET,CfgSettings.local_ip,&addr);

        if ((CfgSettings.local_ip[0] == '\0') || (match_cidr(addr, "\x7F", 8)) || (match_cidr(addr, "\0", 8)) || match_cidr(addr,"10.0.0.0", 8) ||
            match_cidr(addr,"172.16.0.0", 12)) 
                            memset (vhost, '\0', sizeof(vhost));
        else
            snprintf (vhost, sizeof(vhost), "%s", CfgSettings.local_ip);

	
        ret = Connect (statsock, HTTP_ADDR, vhost, HTTP_PORT);
	
        if (!ret)
		{
			dl	= dlink_find_delete (statsock, &sockets);
			dlink_free (dl);
			free (statsock);
			return;
		}

		return;
	}
	return;
}

