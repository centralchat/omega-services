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
 *    $Id: g_ipbl.c 2310 2012-01-30 01:59:42Z jer $
 */
 

#include "stdinc.h"
#include "server.h"

 
#define NAME	"g_ipbl"
#define	AUTHOR	"Twitch"
#define VERSION	"$Id: g_ipbl.c 2310 2012-01-30 01:59:42Z jer $"

#define HTTP_ADDR  "omega-services.org"
#define HTTP_SERV  "omega-services.org"
#define HTTP_PORT  80


#define IPBL_VER 2
typedef struct _ipbl_que {
	char *ip;
	char *reason;
} IPBL;

Socket *ipbl_sock;


int Module_Open();
void Module_Close();

static void	evh_first_run	(int, char **);
static void	sock_read		(Socket *, int);
static void	sock_write		(void *);

void ipbl_check_ip(User *);
static void evh_ipblcon(int, User*);
static void ipbl_response(Link *, int, char **);
static void ipbl_error(Link *, int, char **);

dlink_list ipbl_que;


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Open, Module_Close);


int Module_Open()
{
	AddEvent ("USERCONN", evh_ipblcon);
	AddServCmd("IPBL", ipbl_response);	
	AddServCmd("IPBLERROR", ipbl_error);
	return MOD_CONT;
}

void Module_Close()
{
	dlink_node *dl;

	DelEvent("USERCON", evh_ipblcon);
	DelServCmd("IPBL", ipbl_response);
	
	if ((ipbl_sock = find_socket ("ipblsock")))
	{
		close (ipbl_sock->sd);
		dl	= dlink_find_delete (ipbl_sock, &sockets);
		dlink_free (dl);
		free (ipbl_sock);
	}
	return;
}


static void ipbl_error(Link *li, int ac, char **av) {
	return;
}

char *get_bl_type(char *type)
{
	int index = 0; 
	if (*type)
	{		
		index = atoi(type);
		
		if ((index < 0) || (index > 11))
				index = 0;
	}
	
	const char *BLType[] = {
		"Unknown",
		"DDoS Drone",
		"IRC Drone",
		"Socks Proxy",
		"HTTP Proxy",
		"WinGate Proxy",
		"HTTP Post Proxy",
		"Brute Force Attacker",
		"Compromised Router/Gateway",
		"Bottler",
		"Unknown spambot or drone",
		"Malicious User",
		NULL 
	};
	return (char *) BLType[index];
}

static void	sock_read	(Socket *s, int e)
{
	char	*msg;
	char	*line;

	while (s->fullLines > 0)
	{
		line	= GetLine (s);
		
		if (line == NULL)
				break;
								
			parse (line);

            memcounts.buflinesize   -= sizeof(line);
            memcounts.buflinecnt--;
            free (line);	
	}
	return;
}

static void evh_ipblcon(int ac, User *u)
{
		char				vhost[63];
		unsigned char		addr[16];
		
		if (sync_state != RUNNING)
				return;
				
		inet_pton(AF_INET,u->ip,&addr);
		
		if (strcasecmp(u->host, "data.searchirc.org")==0)
			return;

		if ((match_cidr(addr, "\x7F", 8)) || (match_cidr(addr, "\0", 8)) || match_cidr(addr,"10.0.0.0", 8) ||
            match_cidr(addr,"172.16.0.0", 12))
                        return;
						
		ipbl_check_ip(u);
		return;
}

static void sock_write	(void *args)
{
		Socket *s = args;
		dlink_node *dl, *tdl;
		char *post_info;
		
		DLINK_FOREACH_SAFE(dl, tdl, ipbl_que.head)
		{
				post_info = dl->data;
				
				if (!*post_info)
						break;
				
				alog(DEBUG3,"IPBL: %s", post_info);
				
				if (sendto_socket (s, "%s", post_info) != 0)
						s->flags |= SOCK_DEAD;
				
				s->flags	&= ~SOCK_CONNECTING;
				s->flags	&= ~SOCK_WRITE;
				break;
		}
		dl = dlink_find_delete(post_info, &ipbl_que);
		dlink_free(dl);
		free(post_info);	
		return;
}

static void ipbl_response(Link *li, int ac, char **av)
{
	dlink_node *dl;
	char *params[512];
	int n;
	
	if (ac < 3)
		return;
		
	if (strcasecmp(av[1],"Error")==0)
		return;
		
	sendto_logchan("IPBL: %s %s (%s)", av[1], get_bl_type(av[3]), av[2]);
	ircd_xline("G", (HasId(s_Guardian))? s_Guardian->uid : s_Guardian->nick, av[1], 0, "Your IP was found on a blacklist. (Reason: %s) For more information please visit http://omega-services.org/?mod=ipbl&entry=%s", get_bl_type(av[3]), av[2]);
	return;
}

void ipbl_check_ip(User *u)
{
	Socket *s;
	
	char *data;
	
	int ret;
	dlink_node *dl;
	char content[2048];
	char headers[2048];

	evh_first_run	(0, NULL);

	if (!(data = (char *)malloc(sizeof(char) * 6000)))
			return;
		
	dl = dlink_create();
	
	memset (content, '\0', 2048);
	memset (headers, '\0', 2048);
	
	snprintf (content, 2048, "protver=%d&check=1&ip=%s&host=%s",IPBL_VER, (u->ip)? u->ip : "none", (u->host)? u->host : "none");	
	snprintf (headers, 2048, "User-Agent: Omega/%s Servername/%s\nHost: %s\nContent-Length: %d\nContent-Type: application/x-www-form-urlencoded", VERSION_STRING_DOTTED, CfgSettings.servername, HTTP_SERV, (int)strlen(content));
	snprintf(data, 6000, "POST /bl/api.php HTTP/1.1\n%s\n\n%s\r\n\r\n", headers, content);
	dlink_add_tail(data, dl, &ipbl_que);	
	return;
}

static void	evh_first_run	(int ac, char **av)  {
		char				vhost[63];
		unsigned char		addr[16];
		int					ret;
		dlink_node			*dl;

		ipbl_sock	= add_new_socket ((void *) sock_read, sock_write);
		ipbl_sock->flags	|= SOCK_CONNECTING | SOCK_WRITE;
		strlcpy (ipbl_sock->name, "ipblsock", sizeof(ipbl_sock->name));
		
		inet_pton(AF_INET,CfgSettings.local_ip,&addr);

        if ((CfgSettings.local_ip[0] == '\0') || (match_cidr(addr, "\x7F", 8)) || (match_cidr(addr, "\0", 8)) || match_cidr(addr,"10.0.0.0", 8) ||
            match_cidr(addr,"172.16.0.0", 12)) 
                            memset (vhost, '\0', sizeof(vhost));
        else
            		    snprintf (vhost, sizeof(vhost), "%s", CfgSettings.local_ip);
			
		ret = Connect (ipbl_sock, HTTP_ADDR, vhost, HTTP_PORT);
		
		if (!ret)
		{
			dl	= dlink_find_delete (ipbl_sock, &sockets);
			dlink_free (dl);
			free (ipbl_sock);
			return;
		}
}
