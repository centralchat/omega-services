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
 *    $Id: socketengine.c 2359 2012-02-15 20:24:56Z twitch $
 */


#include "stdinc.h"
#include "server.h"

static char	sock_getbyte (Socket *);
static int	    sock_readline (Socket *);
void	sendq_add	(Socket *, char *, int, int);
int	    sendq_flush 	(Socket *);
static char *socket_flags_to_string (Socket *);

int		maxfds;

#ifdef USE_KQUEUE
static struct kevent    *kqlst;
static int				kqmax;
static int				kqoff;
static int				kq;

static void socket_exit  (void *);

void init_kqueue (void);

void init_kqueue (void)
{
	kq	= kqueue ();
	kqmax	= getdtablesize ();
	kqlst	= malloc (sizeof (struct kevent) * kqmax);
	return;
}
#endif

/* Read Buffer */
static char readBuffer[MAXLEN + 1];
static int  data_in_buffer  = 0;



//these are safe to declare as definitions are
//are safe for machines with/o pthreading - Twitch
Thread* SocketThread;

/*****************************************/

Socket *add_new_socket(void (*read)(Socket*, int),void (*write)(void*))
{
        Socket  	*s;
        dlink_node	*dl;

        if (!(s = new_socket()))
				return NULL;
		
		if (read)
			s->read_callback = read;
	
		if (write)
			s->write_callback = write;

        return s;
}

Socket * new_socket ()
{
    Socket  	*s;
	dlink_node	*dl;

    s = (Socket *) malloc(sizeof(Socket));

	memset (s->name, '\0', sizeof(s->name));

    s->sd           = -1;
    s->fullLines    =  0;

	s->flags		= 0x0;
	s->user		    = NULL;

    s->read_callback    = NULL;
    s->write_callback   = NULL;
    s->error_callback   = NULL;

#ifdef HAVE_GNUTLS
    s->tls_enabled      = 0;
    s->tls_session      = NULL;
#endif

    s->timeout          = 0;
    s->connected        = time (NULL);

    s->mbuffer.head = s->mbuffer.tail   = NULL;
	s->sendq.head	= s->sendq.tail		= NULL;

	s->read_buffer[0]	= '\0';

	dl	= dlink_create ();

	dlink_add_tail	(s, dl, &sockets);

	memcounts.socketcnt++;
	memcounts.socketsize	+= sizeof(s);

    return s;
}

Socket *find_socket (char *name)
{
	Socket		*s;
	dlink_node	*dl;

	DLINK_FOREACH (dl, sockets.head)
	{
		s	= (Socket *) dl->data;

		if (s->name[0] != '\0')
			if ((strcasecmp (s->name, name)) == 0)
				return s;
	}

	return NULL;
}


/************************************************/

#ifdef HAVE_GETADDRINFO
struct addrinfo	* gethostinfo (char const *host, int port);

int Connect(Socket* s, char *server, char *vhost, int port)
{
	struct addrinfo	*hostres, *res;
	struct addrinfo *bindres = NULL;
	int optval, flags, ret;

        s->sd   = -1;

	hostres		= gethostinfo (server, port);

	if (hostres == NULL)
	{
		fprintf (stderr, "Lookup failure for hostname %s\n", server);
        return -1;
	}

	if (*vhost != '\0')
	{
		bindres	= gethostinfo (vhost, 0);

		if (bindres == NULL)
		{
			fprintf (stdout, "Connection Failed! (Lookup failure for vhost %s)\n", vhost);
            return -1;
		}
	}

#ifdef HAVE_GNUTLS
    if (s->tls_enabled)
    {
        //gnutls_anon_allocate_client_credentials (&s->tls_anoncred);
        //gnutls_certificate_allocate_credentials (&s->tls_x509_cred);
        gnutls_init (&s->tls_session, GNUTLS_CLIENT);

        /* Use default priorities */
        //gnutls_priority_set_direct (s->tls_session, "PERFORMANCE:+ANON-DH:!ARCFOUR-128", NULL);
        gnutls_set_default_priority (s->tls_session);
        /* put the anonymous credentials to the current session */
        //gnutls_credentials_set (s->tls_session, GNUTLS_CRD_ANON, tls_anoncred);

        gnutls_credentials_set (s->tls_session, GNUTLS_CRD_CERTIFICATE, tls_x509_cred);

        //gnutls_transport_set_push_function(s->tls_session, gnutls_push_wrapper);
        //gnutls_transport_set_pull_function(s->tls_session, gnutls_pull_wrapper);
    }
#endif

	for (res = hostres; res != NULL; res = res->ai_next)
	{
		s->sd	= socket (res->ai_family, res->ai_socktype, res->ai_protocol);

		if (s->sd < 0)
			continue;

		optval	= 1;

		setsockopt (s->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

		if (!(s->flags & SOCK_UPLINK))
		{
            /* XXX Until I determine the proper method for handling TLS and non-blocking */
#ifdef HAVE_GNUTLS
            if (!s->tls_enabled) {
#endif
			    flags	= fcntl (s->sd, F_GETFL, 0);
			    flags	|= O_NONBLOCK;
			    (void) fcntl (s->sd, F_SETFL, flags);
#ifdef HAVE_GNUTLS
            }
#endif
		}

		if (bindres != NULL)
		{
			if (bind (s->sd, bindres->ai_addr, bindres->ai_addrlen) < 0)
			{
				fprintf (stdout, "Connect Failed! (Unable to bind to %s)\n", vhost);
				perror("Bind");	
				s->sd	= -1;
				return -1;
			}
		}

		if ((connect (s->sd, res->ai_addr, res->ai_addrlen)) != 0)
		{
			if (!(flags & O_NONBLOCK))
			{
				fprintf (stdout, "Connect Failed! (%s)\n", strerror(errno));
				s->sd	= -1;
            			return -1;
			}
		}
		if (s->sd > maxfds)
			maxfds	= s->sd;

		if (!(s->flags & SOCK_UPLINK))
			s->flags |= SOCK_WRITE;
		else {
            if (!(s->flags & SOCK_READ) || !(s->flags & SOCK_WRITE))
			    s->flags |= SOCK_READ;
        }

#ifdef HAVE_GNUTLS
        if (s->tls_enabled) {
            gnutls_transport_set_ptr (s->tls_session, (gnutls_transport_ptr_t) s->sd);

            ret = gnutls_handshake (s->tls_session);

            if (ret < 0) {
                if (ret != GNUTLS_E_AGAIN && ret != GNUTLS_E_INTERRUPTED) {
                    fprintf (stderr, "*** TLS Handshake failed\n");
                    gnutls_perror(ret);

                alog (2, "IO/TLS: %s :Handshake failed", s->name);

                close (s->sd);
                s->sd   = -1;
                s->flags    |= SOCK_DEAD;

                if (s->tls_session) {
                    gnutls_deinit (s->tls_session);
                    s->tls_session  = NULL;
                }
                /*
                if (s->tls_x509_cred) {
                    gnutls_certificate_free_credentials (s->tls_x509_cred);
                    s->tls_x509_cred    = NULL;
                }
                */

                return -1;
                }
                else {
                    alog (2, "IO/TLS: %s :Requested try again", s->name);

                    if (gnutls_record_get_direction(s->tls_session) == 0)
                        alog (2, "IO/TLS: %s :Wants read() again", s->name);
                    else
                        alog (2, "IO/TLS: %s :Wants write() again", s->name);

                    //gnutls_handshake (s->tls_session);
                }
            }

            fprintf (stdout, "TLS Handshake complete\n");
            alog (2, "IO/TLS: %s: Handshake complete.", s->name);
        }
#endif

		break;
	}

	freeaddrinfo (hostres);


	if (bindres != NULL)
		freeaddrinfo (bindres);

    return s->sd;
}
#else
void Connect(Socket* s, char *server, char *vhost, int port)
{
	struct hostent *he = NULL;
	struct sockaddr_in my_sin;
	struct in_addr * in;


	if((s->sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Unable to bind Socket\n");
		exit(1);
	}



	if (*vhost != '\0')
	{
		memset ((void *) &my_sin, '\0', sizeof(my_sin));
		my_sin.sin_family	= AF_INET;

		if ((he = gethostbyname (vhost)) == NULL)
		{
			printf (" Failed! (Invalid Address)\n");
			exit (1);
		}

		in	= (struct in_addr *) (he->h_addr_list[0]);
		my_sin.sin_addr.s_addr	= in->s_addr;
		my_sin.sin_port = 0;

		if (bind (s->sd, (struct sockaddr *) &my_sin, sizeof(my_sin)) < 0)
		{
			printf ("Connection Failed! (Unable to bind to %s)\n", vhost);
			exit (1);
		}
	}

	he = gethostbyname(CfgSettings.uplink);
	if (!he)
	{
		printf ("Invalid Address\n");
		exit (1);
	}

	my_sin.sin_family = AF_INET;
	my_sin.sin_port = htons(port);	/* port, htons converts integers to network short */
	my_sin.sin_addr = *((struct in_addr *) he->h_addr);
	memset(my_sin.sin_zero, 0, sizeof(my_sin.sin_zero)); /* sin_zero is 8 bytes used for padding */

	if(connect(s->sd, (struct sockaddr *)&my_sin, sizeof(struct sockaddr)) == -1)
	{
		perror("Connect\n");
		exit(1);
	}

	return s->sd;


}
#endif

#ifdef HAVE_GETADDRINFO
int Listen (Socket *s, char *address, int port) {
    struct addrinfo     *hostres, *res;
    struct addrinfo     *bindres = NULL;
    int                 optval, flags;

    hostres     = gethostinfo (address, port);

    if (hostres == NULL) {
        alog (LOG_ERROR, "Lookup failure for hostname %s", address);
        return -1;
    }

    for (res = hostres; res != NULL; res = res->ai_next) {
        s->sd   = socket (res->ai_family, res->ai_socktype, res->ai_protocol);

        if (s->sd < 0)
            continue;

        optval  = 1;
        setsockopt (s->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

        if ((bind (s->sd, res->ai_addr, res->ai_addrlen)) != 0) {
            alog (LOG_ERROR, "Unable to bind to %s: %s", address, strerror(errno));
            s->sd   = -1;
            return -1;
        }

        if ((listen (s->sd, 5)) != 0) {
            alog (LOG_ERROR, "Unable to listen on %s: %s", address, strerror(errno));
            close (s->sd);
            s->sd   = -1;
            return -1;
        }

        if (s->sd > maxfds)
            maxfds  = s->sd;

        s->flags    |= SOCK_LISTEN|SOCK_READ;
        break;
    }

    freeaddrinfo (hostres);

    return s->sd;
}

#else
int Listen (Socket *s, char *address, int port) {
    int                 optval, flags;
    struct sockaddr_in  serv_addr;
    struct hostent      *bind_addr;

    bind_addr   = gethostbyname (address);

    if (bind_addr == NULL) {
        alog (LOG_ERROR, "Unable to get info for address: %s", address);
        return -1;
    }

    if ((s->sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        alog (LOG_ERROR, "Unable to create socket: %s", strerror(errno));
        return -1;
    }

    bzero ((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family    = AF_INET;
    serv_addr.sin_port      = htons(port);
    bcopy ((char *) bind_addr->h_addr, (char *) &serv_addr.sin_addr.s_addr, bind_addr->h_length);

    optval  = 1;
    setsockopt (s->sd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval));

    if (bind (s->sd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        alog (LOG_ERROR, "Unable to bind to address %s: %s", address, strerror(errno));
        close (s->sd);
        return -1;
    }

    if (listen (s->sd, 5) < 0) {
        alog (LOG_ERROR, "Unable to listen on socket: %s", strerror(errno));
        return -1;
    }

    s->flags    |= SOCK_LISTEN|SOCK_READ;
    return s->sd;
}
#endif


/************************************************/


struct in_addr *GetHost(char *address)
{
	return NULL;
}


/************************************************/

int receive(Socket *s)
{
	dlink_node	*dl, *tdl;
	int		rc;
	int 		modque = 0;
	struct timeval  timeout;

#ifdef USE_KQUEUE
	int				kq, nev, i;
	struct kevent	chlist[2];
	struct kevent	evlist[2];

	if ((kq = kqueue()) == -1)
	{
		perror("kqueue");
		exit (EXIT_FAILURE);
	}
#endif

#ifndef USE_KQUEUE
		FD_ZERO(&wfdset);
		FD_ZERO(&rfdset);
		FD_ZERO(&errfdset);

		DLINK_FOREACH_SAFE (dl, tdl, sockets.head)
		{
			s	= dl->data;

            /* Enforce socket timeouts */
            if (s->timeout > 0) {
                if ((time(NULL) - s->connected) > s->timeout) {
                    s->flags    |= SOCK_DEAD;
                }
            }

            alog (LOG_DEBUG3, "fd: %d name: %s flags: %s", s->sd, s->name, socket_flags_to_string (s));

			// Clean out dead sockets
			if (((s->flags & SOCK_DEAD) || (s->sd == -1) 
				||	((s->flags & SOCK_WRITE) && (time(NULL) - s->connected) > 120)) && !(s->flags & SOCK_UPLINK))
			{
					if (s->sd > 0)
						close(s->sd);

#ifdef HAVE_GNUTLS
                    if (s->tls_session) {
                        fprintf (stdout, "TLS - Destroying TLS session\n");
                        if (s->tls_session) {
                            gnutls_deinit (s->tls_session);
                            s->tls_session  = NULL;
                        }
                        /*
                        if (s->tls_x509_cred) {
                            gnutls_certificate_free_credentials (s->tls_x509_cred);
                            s->tls_x509_cred    = NULL;
                        }
                        */
                    }
#endif

                    if ((strcasecmp(s->name, servsock->name) == 0))
                        continue;

                    /* If this callback happened to be in a module there is potential for a core - Jer */
                    if (s->error_callback) {
                        alog (LOG_DEBUG2, "Calling error_callback at %p for %s", s->error_callback, s->name);
                        s->error_callback (s);
                    }
							
					dl = dlink_find_delete(s,&sockets);
					dlink_free(dl);
					free(s);
					continue;
			}
			else if (s->flags & SOCK_CONNECTING) {
				if (s->sd <= 0)
					continue;
                //alog (LOG_DEBUG3, "Adding fd %d (%s) to select loop (CONNECTING)", s->sd, s->name);
				if (s->flags & SOCK_DCCOUT)
					FD_SET (s->sd, &wfdset);
				else if (s->flags & SOCK_DCCIN)
					FD_SET (s->sd, &rfdset);
				else if (s->flags & SOCK_UPLINK)
					FD_SET (s->sd, &wfdset);
				else if (s->flags & SOCK_READ)
					FD_SET (s->sd, &rfdset);
				else
					FD_SET (s->sd, &wfdset);
                  alog(LOG_SOCKET, "Socket is in: %s (Name: %s)(Fd: %d)", (s->flags & SOCK_READ)? "Read" : "Write", s->name, (int)s->sd);
 
			}
			else if (s->flags & SOCK_WRITE) {
				if (s->sd > 0)
                	FD_SET (s->sd, &wfdset);
			} else {
                //alog (LOG_DEBUG3, "Adding fd %d (%s) to select loop (READ)", s->sd, s->name);
				if (s->sd > 0)
					FD_SET (s->sd, &rfdset);
            }
		}


		timeout.tv_sec = 0;
		timeout.tv_usec = 250 * 1000;
                
		rc = select (maxfds + 1, &rfdset, &wfdset, NULL, &timeout);

		if (rc > 0) // This indicates we have fd's ready for read/write
		{
				DLINK_FOREACH_SAFE (dl, tdl, sockets.head)
				{
					s	= (Socket *) dl->data;

                    if ((!s) || (s->sd <= 0))
                        continue;

					if (FD_ISSET (s->sd, &rfdset))
					{
						if ((s->flags & SOCK_CONNECTING) || (s->flags & SOCK_LISTEN))
						{
							    if (s->read_callback) {
								alog (LOG_DEBUG3, "Calling read_callback at %p for %s", s->read_callback, s->name);
											    s->read_callback(s,EVENT_READ);
							    }
							    continue;
						}

						sock_readline (s);
                        if (s->read_callback) {
                            alog (LOG_DEBUG3, "Calling read_callback at %p for %s", s->read_callback, s->name);
						    s->read_callback(s,EVENT_READ);
                        }

                        continue;
					}

					if (FD_ISSET (s->sd, &wfdset))
					{
					    if (s->write_callback) {
                            alog (LOG_DEBUG3, "Calling write_callback at %p for %s", s->write_callback, s->name);
						    s->write_callback(s);
                        }

                        continue;
					}
				}
			return 0;
		}
		else
		{
			if (rc == 0)    // Again 0 is okay
				return 0;

			else if ((rc == -1) && (errno == EINTR)) // Interrupted system call, we got a signal, simply restart the select loop
				return 0;
			else
			{
			            //do not progress here, we are shutting down. 
                        //this will prevent invalid socket to caught as signal
                        //is shutting us down.
                        if (sync_state == SHUTDOWN)
                                return 0;
                
				/* Select() error, handle */
				alog(4, "Select Error: [%d] %s\n", errno, strerror(errno));
                DLINK_FOREACH (dl, sockets.head) {
                    s   = (Socket *) dl->data;

                    alog (4, "Sockets: Socket[%d]: %s\n", s->sd, s->name);
                }
				exit (-1);
			}
		}
#else

		DLINK_FOREACH_SAFE (dl, tdl, sockets.head)
		{
			s	= (Socket *) dl->data;

			if (s->sd == -1)
				continue;

			if (s->flags & SOCK_CONNECTING)
			{
				if (s->flags & SOCK_DCCOUT)
					;
			}
		}

		DLINK_FOREACH (dl, connected_sockets.head)
		{
			s	= dl->data;

			if (s->sd != -1)
				EV_SET(&chlist[0], s->sd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
			else if (s->dead)
				EV_SET(&chlist[0], s->sd, 0, EV_DELETE, 0, 0, 0);
		}

		for (;;)
		{
			nev	= kevent (kq, chlist, 1, evlist, 1, NULL);

			if (nev < 0)
			{
				perror ("kevent()");
				exit (EXIT_FAILURE);
			}

			if (nev > 0)
			{
				if (evlist[0].flags & EV_EOF)
					exit (EXIT_FAILURE);
				break;
			}
		}

		if (nev == 0)
			continue;

		for (i = 0; i < nev; i++)
		{
			if (evlist[i].flags & EV_ERROR)
			{
				fprintf (stderr, "EV_ERROR: %s (%d)\n", strerror(evlist[i].data), evlist[i].ident);
				exit (EXIT_FAILURE);
			}

			DLINK_FOREACH (dl, connected_sockets.head)
			{
				s	= dl->data;

				if (evlist[i].ident == s->sd)
				{
					MyRead (s);
					HandleSockEvent (EVENT_READ);
				}
			}
		}
#endif /* USE_KQUEUE */
	/* If we reach here something has broke us out of the select loop */
	return 0;
}

static char *socket_flags_to_string (Socket *s) {
    static char string[4096];

    string[0]   = '\0';

    if (s->flags & SOCK_UPLINK) {
        if (string[0] != '\0')
            strlcat (string, ",UPLINK", sizeof(string));
        else
            strlcpy (string, "UPLINK", sizeof(string));
    }

    if (s->flags & SOCK_WRITE) {
        if (string[0] != '\0')
            strlcat (string, ",WRITE", sizeof(string));
        else
            strlcpy (string, "WRITE", sizeof(string));
    }

    if (s->flags & SOCK_READ) {
        if (string[0] != '\0')
            strlcat (string, ",READ", sizeof(string));
        else
            strlcpy (string, "READ", sizeof(string));
    }

    if (s->flags & SOCK_DEAD) {
        if (string[0] != '\0')
            strlcat (string, ",DEAD", sizeof(string));
        else
            strlcpy (string, "DEAD", sizeof(string));
    }

    if (s->flags & SOCK_LISTEN) {
        if (string[0] != '\0')
            strlcat (string, ",LISTEN", sizeof(string));
        else
            strlcpy (string, "LISTEN", sizeof(string));
    }

    if (s->flags & SOCK_CONNECTING) {
        if (string[0] != '\0')
            strlcat (string, ",CONNECTING", sizeof(string));
        else
            strlcpy (string, "CONNECTING", sizeof(string));
    }

    return string;
}

/************************************************/

int send_line(char *fmt,...)
{
	char	tmp[1024];

	int n;
	va_list args;

	if (!servsock)
		return 0;

	if ((!servsock->sd) || (servsock->sd <= 0))
		return 0;

	if (!fmt)
		return -1;

    if (servsock->flags & SOCK_DEAD)
        return 0;

	memset(tmp,0,sizeof(tmp));

	va_start(args,fmt);
	vsnprintf(tmp,1024,fmt,args);
	va_end (args);

	alog (LOG_DEBUG2, "SEND: %s", tmp);

	if (!strstr(tmp,"\r")) { strcat(tmp,"\r"); }
	if (!strstr(tmp,"\n")) { strcat(tmp,"\n"); }


	if (strlen(tmp) > 512)
		return -1;

#if 0
	/**
	 ** This is our example of throttling
	 ** Allows 10 lines every 2 seconds
	 */

	thisline	= time(NULL);

	if (!lastline)
		lastline	= thisline;

	if ((thisline - lastline) < 2)
	{
		if (lines > 10)
		{
			sendq_add (sock, tmp, strlen(tmp), 0);
			return;
		}
		else
			lines++;
	}
	else
	{
		lastline = thisline;
		lines = 0;
	}

#endif


	/* This should prevent sending from blocking */
	n	= sendq_flush (servsock);

	if (n == -1)
	{
		if (errno != EAGAIN)
		{
            alog (LOG_ERROR, "Write error on %s: (%d) %s", servsock->name, errno, strerror(errno));
            servsock->flags |= SOCK_DEAD;
			uplink_cleanup (NULL, 0);
            AddTimedEvent ("Connect Uplink", ev_connectuplink, 60);
			/* Catch errors and close the uplink socket */
			return 1;
		}
		else
		{
			sendq_add (servsock, tmp, strlen(tmp), 0);
			return 0;
		}
	}
	else if (n == 0)
	{
		sendq_add (servsock, tmp, strlen(tmp), 0);
		return 0;
	}

#ifdef HAVE_GNUTLS
	if (servsock->tls_enabled && servsock->tls_session)
	{
		if ((n = gnutls_record_send (servsock->tls_session, tmp, strlen(tmp))) == -1)
		{
        		if (errno != EAGAIN)
        		{
            			uplink_cleanup (NULL, 0);
            			AddTimedEvent ("Connect Uplink", ev_connectuplink, 60);
            			/* Catch errors and close the uplink socket */
            			return 1;
        		}
        		else
        		{
            			sendq_add (servsock, tmp, strlen(tmp), 0);
            			return 0;
			}
		}

        sentbytes += n;

       	if (n != strlen(tmp))
               	sendq_add (servsock, tmp, strlen(tmp), n);

       	return 0;
    }
#endif

	if ((n = write (servsock->sd, tmp, strlen(tmp))) == -1)
	{
		if (errno != EAGAIN)
		{
            alog (LOG_ERROR, "Write error on %s: (%d) %s", servsock->name, errno, strerror(errno));
            servsock->flags |= SOCK_DEAD;
			uplink_cleanup (NULL, 0);
            AddTimedEvent ("Connect Uplink", ev_connectuplink, 60);
			/* Catch errors and close the uplink socket */
			return 1;
		}
		else
		{
			sendq_add (servsock, tmp, strlen(tmp), 0);
			return 0;
		}
	}
       sentbytes += n;

	if (n != strlen(tmp))
		sendq_add (servsock, tmp, strlen(tmp), n);

	return 0;
}

static char sock_getbyte (Socket *s)
{
	static char		*c = readBuffer;
	char			rc;
	int				n;
    static int      readBuffer_cnt = 0;

	if (data_in_buffer == 0)
	{
        memset (readBuffer, '\0', sizeof(readBuffer));
#ifdef HAVE_GNUTLS
        if (s->tls_enabled) {
            n = gnutls_record_recv (s->tls_session, readBuffer, MAXLEN);

            if ((n == GNUTLS_E_INTERRUPTED) || (n == GNUTLS_E_AGAIN))
                return '\0';    /* Come around again */

            if (n <= 0)
                alog (LOG_ERROR, "IO/TLS: %s: %s", s->name, gnutls_strerror(n));
        }
        else
#endif
		    n = read (s->sd, readBuffer, MAXLEN);

		if (n <= 0)
		{
			if (s->flags & SOCK_UPLINK)
			{
            		       s->flags |= SOCK_DEAD;
				uplink_cleanup (NULL, 0);
				AddTimedEvent ("Connect Uplink", ev_connectuplink, 60);
				return '\0';
			}
			else
			{
                alog (LOG_ERROR, "Read error on %s: (%d) %s", s->name, n, strerror(errno));

                if (s->error_callback)
                    s->error_callback (s);

				shutdown (s->sd, SHUT_RDWR);
				close (s->sd);

				s->flags	|= SOCK_DEAD;

				return '\0';
			}
		}

		readBuffer[n] = '\0';
        /*
        alog (2, "IO: %s: read: %d", s->name, n);
        alog (2, "IO: %s: message: %s", s->name, readBuffer);
        alog (2, "IO: %s: len: %d", s->name, strlen(readBuffer));
        */
	         recvbytes += n;
	
		c = readBuffer;
		data_in_buffer = 1;

	}

	if (*c == '\0')
	{
		data_in_buffer = 0;
		readBuffer[0] = '\0';

		return *c;
	}
    
	rc = *c;
	c++;
	return rc;
}

static int sock_readline (Socket *s)
{
	char	message[MAXLEN + 1];
	int				i;
	char			c;
	MessageBuffer	*m;
	dlink_node		*dl;

	i = 0;

	if (s->read_buffer[0] != '\0')
	{
        alog (LOG_DEBUG2, "IO: %s read_buffer: %s", s->name, s->read_buffer);
		strlcpy (message, s->read_buffer, (MAXLEN + 1));
		i = strlen(message);
		message[i] = '\0';
		s->read_buffer[0] = '\0';
	}

	while (TRUE)
	{
		c = sock_getbyte (s);

		if (s->flags & SOCK_DEAD) {
            if (s->read_buffer != '\0')
                alog (LOG_DEBUG2, "read buffer: %s\n", s->read_buffer);
			return 1;
        }

		if (c == '\0')
		{
			if (i > 0)
			{
				message[i] = '\0';
				strlcpy (s->read_buffer, message, MAXLEN + 1);
                break;
                //continue;
			}

			break;
		}

		if ((c == '\r') || (c == '\n'))
		{
			if (i > 0)
			{
				message[i] = '\0';

				dl	= dlink_create ();
				m	= (MessageBuffer *) malloc (sizeof(MessageBuffer));

				memset (m->message, '\0', sizeof(m->message));

				memcounts.msgbufcnt++;
				memcounts.msgbufsize	+= sizeof(MessageBuffer);
				memcounts.buflinecnt++;
				memcounts.buflinesize += sizeof(m->message);

				strlcpy (m->message, message, sizeof(m->message));

				s->fullLines++;
				dlink_add_tail (m, dl, &s->mbuffer);

//                alog (2, "IO/IN: %s: %s", s->name, m->message);

				i = 0;
			}

			continue;
		}

		message[i] = c;
		i++;
	}

	return 0;
}

/*************************************************/
/**
* Return a pointer to our buffered lines
*/

char *GetLine (Socket *s)
{
    dlink_node      *dl, *tdl;
    char            *line;
    MessageBuffer   *m;

    //don't even try to continue
    if (s->fullLines == 0)
        return NULL;

    line = (char *) malloc(MAXLEN + 1);

    if (!line)
    {
	alog(3,"Unable to allocate memory in %s",__FILE__);
	return NULL;
    }


    DLINK_FOREACH_SAFE (dl, tdl, s->mbuffer.head)
    {
        m   = dl->data;

        if (!m->message)
        {
            s->fullLines    = 0;
            return NULL;
        }

        memcpy (line, m->message, MAXLEN);

        memcounts.msgbufsize  -= sizeof(MessageBuffer);
        memcounts.msgbufcnt--;

        memcounts.buflinecnt--;
        memcounts.buflinesize -= sizeof(m->message);
 

        dlink_delete (dl, &s->mbuffer);
        dlink_free (dl);
        free(m);
	s->fullLines--;
	return line;
    }

    return NULL;
}

/**
 ** SendQ commands
 */

void sendq_add (Socket *s, char *buf, int len, int pos)
{
	struct sendq	*sq;
	dlink_node		*dl = dlink_create ();

     sq          =   (struct sendq *)malloc(sizeof(struct sendq));

	memcounts.sendqsize	+= sizeof(sq);
	memcounts.sendqcnt++;

	sq->buf		    = strdup (buf);
	sq->len		    = len - pos;
	sq->pos		= pos;
    sq->s           = s->sd;

	dlink_add_tail (sq, dl, &s->sendq);
}

int sendq_flush (Socket *s)
{
	struct sendq	*sq;
	int 			n;
	dlink_node		*dl, *tdl;

	DLINK_FOREACH_SAFE (dl, tdl, s->sendq.head)
	{
		sq	= dl->data;

		if (sq->s > 0) //only write if our fd is valid else just purge stuff out
		{
#ifdef HAVE_GNUTLS
            if (s->tls_enabled && s->tls_session)
                n = gnutls_record_send (s->tls_session, sq->buf + sq->pos, sq->len);
            else
#endif
                n = write (sq->s, sq->buf + sq->pos, sq->len);

			if (n == -1)
			{
				if (errno != EAGAIN)
					return -1;
				return 0;
			}
		}
        if (n == sq->len)
        {
			memcounts.sendqcnt--;
			memcounts.sendqsize -= sizeof(sq);

			dlink_delete (dl, &s->sendq);
			free (sq->buf);
			free (sq);
			dlink_free (dl);
        }
        else
        {
            sq->pos += n;
            sq->len -= n;
            return 0;
        }
    }

    return 1;
}

#ifdef HAVE_GETADDRINFO
/* Stolen from FreeBSD's whois client, modified for Sentinel by W. Campbell */
struct addrinfo * gethostinfo(char const *host, int port)
{
  struct addrinfo hints, *res;
  int error;
  char portbuf[6];

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = 0;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  snprintf(portbuf, 6, "%d", port);
  error = getaddrinfo(host, portbuf, &hints, &res);
  if (error)
  {

    alog(3, "gethostinfo error: %s: %s", host,
         gai_strerror(error));
    return (NULL);
  }
  return (res);
}
#endif


/********************************************/
/**
  *  send_cmd()  Sends a command from source
  *                          to the server.
  *
  *   @param u The source of of the command
  *                      if this is null we will use the
  *                      servername.
  *
  * @param cmd The command to send
  *                        this takes veradic arguments
  *  @param arg Arguments to the command
  *  @param ...  Veradic arguments for CMD
  *  @return void
  *
  * Note this is preferred to be used because it assumes
  *  a sender and makes proper format :)
  */


void send_cmd(char* source,char *cmd,char *arg, ...)
{
	char	tmp[512];
	va_list	args;

	if (!cmd)
		return;

	va_start(args,arg);
	vsnprintf(tmp,512,arg,args);

	if (source)
		send_line(":%s %s :%s",source,cmd,tmp);
	else
		send_line(":%s %s :%s",CfgSettings.servername,cmd,tmp);

	va_end(args);
	memset(tmp,0,sizeof(tmp));

	return;
}


/********************************************/

void uplink_cleanup (void *message, int exitlocal)
{
	int	olduselogchan;

		
	if (!servsock || (servsock->sd == -1))
		return;

	alog (LOG_DEBUG, "Cleaning up uplink related data");

	olduselogchan	= uselogchan;
	uselogchan		= 0;

	exit_remote_users ();

	if (exitlocal)
		exit_local_users (message);

	exit_all_servs ();
	
	if ((sync_state > CONNECTING) || (sync_state == SHUTDOWN))
	{
		if (IRCd.ts6) 
			send_line (":%s SQUIT %s :%s", CfgSettings.sid, CfgSettings.sid, (message)? message : "No Message");
		else
			send_line (":%s SQUIT %s :%s", Omega->me->name, Omega->me->name, (message)? message : "No Message");
	} 
	
	Uplink	= NULL;

	if (Omega->me->uplink)
		Omega->me->uplink	= NULL;

	close (servsock->sd);
	servsock->sd	= -1;
	uselogchan		= olduselogchan;
    servsock->flags &= ~SOCK_DEAD;  // Reset socket dead state

#ifdef HAVE_GNUTLS
    if (servsock->tls_session) {
        fprintf (stdout, "TLS - Uplink Cleanup - Destroying TLS session\n");
        gnutls_deinit (servsock->tls_session);
        servsock->tls_session   = NULL;
    }
    /*
    if (servsock->tls_x509_cred) {
        gnutls_certificate_free_credentials (servsock->tls_x509_cred);
        servsock->tls_x509_cred = NULL;
    }
    */
#endif
}

struct hostent *get_hostname(char *para)
{
  char hostname[MAXSERV];
  char *host;
  struct hostent *local_host = NULL;
  struct hostent *host_ptr;

  if (para == NULL)
  {
    gethostname(hostname, MAXSERV - 1);
    hostname[MAXSERV - 1] = '\0';
    host = hostname;
  }
  else
  {
    host = para;
  }
  host_ptr = gethostbyname(host);
  if (host_ptr != NULL)
  {
    local_host = malloc(sizeof(struct hostent));
    memcpy(local_host, host_ptr, sizeof(struct hostent));
  }

  return local_host;
}


void socket_cleanup()
{
	Socket* s;
	dlink_node *dl, *tdl;
	DLINK_FOREACH_SAFE(dl,tdl,sockets.head)
	{
		s = dl->data;
		if (s->sd > 0)
		{
			    sendq_flush(s);
			    if (s->flags & SOCK_LISTEN)
			        shutdown(s->sd,  2);
			    close(s->sd);
		}
		dlink_delete(dl,&sockets);
        dlink_free (dl);
        free (s);
	}
}

/********************************/
void delete_socket(Socket *s) {
    MessageBuffer   *m;
    dlink_node *dl, *tdl;

    DLINK_FOREACH_SAFE (dl, tdl, s->mbuffer.head)
    {
        m   = dl->data;

        memcounts.msgbufsize  -= sizeof(m);
        memcounts.msgbufcnt--;

        dlink_delete (dl, &s->mbuffer);
        dlink_free (dl);
        free(m);

    }
    dl = dlink_find_delete(s, &sockets);
    s->sd = -1;
    s->flags |= SOCK_DEAD;
    s->error_callback = NULL;
    dlink_free(dl);
    free(s);
}


/* vim: set tabstop=4 shiftwidth=4 expandtab */

