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
 *    $Id: socketengine.h 2204M 2013-08-14 14:01:04Z (local) $
 */

#ifndef __SOCKETENGINE_H__
#define __SOCKETENGINE_H__

//#define MAXLEN 16384
//#define MAXLEN 4096
#define MAXLEN 2048
#define MAXREADBUFFER MAXLEN*120+5

#define SOCK_UPLINK		0x00000001
#define SOCK_DCC			0x00000002
#define SOCK_DCCCONN		0x00000004
#define SOCK_WRITE		0x00000008
#define SOCK_READ			0x00000010
#define SOCK_NOREAD		0x00000020
#define SOCK_DEAD		0x00000040

#define SOCK_CONNECTING	0x00000080
#define SOCK_DCCIN		0x00000100
#define SOCK_DCCOUT		0x00000200

#define SOCK_LISTEN		0x00000400

#define SOCK_ERR_OK 0

#ifdef HAVE_GNUTLS
gnutls_anon_client_credentials_t    tls_anoncred;
gnutls_certificate_credentials_t    tls_x509_cred;
#endif


dlink_list	connected_sockets;
dlink_list	sockets;

time_t		rawtime;
struct tm		*timeinfo;

fd_set wfdset,rfdset,errfdset;

typedef struct
{
	char	message[MAXLEN];
} MessageBuffer;

typedef struct socket_
{
	int	sd;
	int	fullLines;
	int	dead;
	int	flags;

	int 	witems;
	int     ritems;

    int     timeout;
    time_t  connected;

	char	read_buffer[MAXLEN + 1];
	char	name[512];

	dlink_list	mbuffer;
	dlink_list	sendq;

	struct sockaddr_in	*sa;

	User	*user;

	void	(*read_callback)  (struct socket_ *, int);
	void	(*write_callback) (void *);
    void    (*error_callback) (struct socket_ *);

#ifdef HAVE_GNUTLS
    int                                 tls_enabled;
    gnutls_session_t                    tls_session;
    gnutls_anon_client_credentials_t    tls_anoncred;
    gnutls_certificate_credentials_t    tls_x509_cred;
#endif

} Socket;

Socket  	*add_new_socket(void (*read)(Socket*, int),void (*write)(void*));

Socket  	*servsock;
Socket	*se_init 		();
Socket	*new_socket	();
Socket	*find_socket	(char *);

int		receive		(Socket*);
int		send_line		(char *, ...);
int		handle_msg	(char *);
int		Connect		(Socket *, char *, char *, int);
char	*GetLine		(Socket *);
void 	socket_cleanup	();

void	se_HandleEvent	(void *);
struct in_addr *GetHost(char *address);

//sending TOKEN support
char 		*SendToken	(char*,char*);
void 		delete_socket	(Socket *);
void 		uplink_cleanup (void *, int);

struct hostent *get_hostname(char *);

extern struct addrinfo * gethostinfo (char const *, int);

struct sendq {
	char	*buf;
	int		len;
	int		pos;
    	int		s;

    struct sendq    *next;
    struct sendq    *prev;
};

dlink_list	sendq;



#endif //__SOCKETENGINE_H__
