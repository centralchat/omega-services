#include "stdinclude.h"
#include "appinclude.h"


/************************************************************/
/** 
 * Api Safe Calls
 */


int se_receive() {
	if (SocketEngine.receive)
		return SocketEngine.receive();
	return 0;
}


int se_startup() {
	if (SocketEngine.startup)
		return SocketEngine.startup();
	return 0;
}

void se_cleanup() {
	if (SocketEngine.cleanup)
		 SocketEngine.cleanup();
	return;
}


/************************************************************/
/** 
 * Register Functions
 */

void add_se_fnct_receive(int  (*fnct)()) 
{
	SocketEngine.receive = fnct;
}

void add_se_fnct_cleanup(void (*fnct)())
{
	SocketEngine.cleanup = fnct;
}

void add_se_fnct_startup(int  (*fnct)())
{
	SocketEngine.startup = fnct;
}



/************************************************************/
/** 
 * Unregister Functions
 */

void del_se_fnct_receive(int  (*fnct)()) 
{
	SocketEngine.receive = NULL;
}

void del_se_fnct_cleanup(void (*fnct)())
{
	SocketEngine.cleanup = NULL;
}

void del_se_fnct_startup(int  (*fnct)())
{
	SocketEngine.startup = NULL;
}


/************************************************************/
/** 
 * se_init
 */

 void se_init(void)
 {
 	sockets.head = NULL;
 	sockets.tail = NULL;
 }



/************************************************************/
/** 
 * se_listen
 */


#ifdef HAVE_GETADDRINFO
int se_listen (Socket *s, char *address, int port) {
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

        if (s->sd > max_sockets)
            max_sockets  = s->sd;

        s->flags    |= SOCK_LISTEN|SOCK_READ;
        break;
    }

    freeaddrinfo (hostres);
    log_message(LOG_SOCKET, "Listening on %s:%d", address, port);
    return s->sd;
}

#else

int se_listen (Socket *s, char *address, int port) 
{
    int                 optval, flags;
    struct sockaddr_in  serv_addr;
    struct hostent      *bind_addr;

    bind_addr   = gethostbyname (address);

    if (bind_addr == NULL) 
    {
        alog (LOG_ERROR, "Unable to get info for address: %s", address);
        return -1;
    }

    if ((s->sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) 
    {
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
    log_message(LOG_SOCKET, "Listening on %s:%d", address, port);
    return s->sd;
}

#endif


/************************************************************/
/** 
 * gethostinfo
 */

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
    log_message(LOG_ERROR, "gethostinfo error: %s: %s", host, gai_strerror(error));
    return NULL;
  }
  return res;
}
#endif


/************************************************************/

int se_accept(Socket * s) 
{
	int sd;
	struct sockaddr_in client_name;
 	int client_name_len = sizeof(client_name);

	sd = accept(s->sd, (struct sockaddr *)&client_name, &client_name_len);
	int flags = fcntl(sd, F_GETFL, 0);
	fcntl(sd, F_SETFL, flags | O_NONBLOCK);

	return sd;
}

/************************************************************/

int _se_readsock(Socket * s)
{
	if (!s) return -1;

	size_t remaining;

	remaining = R_PATHLEN - s->buffer_len;
	int n = read(s->sd, s->buffer + s->buffer_len, remaining);
	s->buffer_len = s->buffer_len + n;

	return n;
}

/*************************************************************/


#ifdef HAVE_GETADDRINFO
struct addrinfo	* gethostinfo (char const *host, int port);

int se_connect(Socket * s, char *server, char *vhost, int port)
{
	struct addrinfo	*hostres, *res;
	struct addrinfo *bindres = NULL;
	int optval, flags, ret;

    s->sd   = -1;

	hostres		= gethostinfo (server, port);
	if (hostres == NULL)
	{
		log_message(LOG_ERROR,"Lookup failure for hostname %s", server);
        return 0;
	}

	if (*vhost != '\0')
	{
		bindres	= gethostinfo (vhost, 0);
		if (bindres == NULL)
		{
			log_message(LOG_ERROR,"Connection Failed! (Lookup failure for vhost %s)", vhost);
            return 0;
		}
	}

	for (res = hostres; res != NULL; res = res->ai_next)
	{
		s->sd	= socket (res->ai_family, res->ai_socktype, res->ai_protocol);

		if (s->sd < 0)
			continue;

		optval	= 1;

		setsockopt (s->sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));

		if (!(s->flags & SOCK_UPLINK))
		{         
		    flags	 = fcntl (s->sd, F_GETFL, 0);
		    flags	|= O_NONBLOCK;
		    (void) fcntl (s->sd, F_SETFL, flags);
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
				log_message(LOG_ERROR, "Connect Failed! (%s)\n", strerror(errno));
				s->sd	= -1;
    			return -1;
			}
		}
		if (s->sd > max_sockets)
			max_sockets	= s->sd;

		if (!(s->flags & SOCK_UPLINK))
		{
			s->flags |= SOCK_WRITE;
		}
		else 
		{
            if (!(s->flags & SOCK_READ) || !(s->flags & SOCK_WRITE))
			    s->flags |= SOCK_READ;
        }
		break;
	}

	freeaddrinfo (hostres);

	if (bindres != NULL)
		freeaddrinfo (bindres);

    return s->sd;
}
#else

int se_connect(Socket* s, char *server, char *vhost, int port)
{
	struct hostent *he = NULL;
	struct sockaddr_in my_sin;
	struct in_addr * in;


	if((s->sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Unable to bind Socket\n");
		return -1;
	}

	if (*vhost != '\0')
	{
		memset ((void *) &my_sin, '\0', sizeof(my_sin));
		my_sin.sin_family	= AF_INET;

		if ((he = gethostbyname (vhost)) == NULL)
		{
			printf (" Failed! (Invalid Address)\n");
			return -1;
		}

		in	= (struct in_addr *) (he->h_addr_list[0]);
		my_sin.sin_addr.s_addr	= in->s_addr;
		my_sin.sin_port = 0;

		if (bind (s->sd, (struct sockaddr *) &my_sin, sizeof(my_sin)) < 0)
		{
			printf ("Connection Failed! (Unable to bind to %s)\n", vhost);
			return -1;
		}
	}

	he = gethostbyname(CfgSettings.uplink);
	if (!he)
	{
		printf ("Invalid Address\n");
		return -1;
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
