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

/************************************************************/

int se_read(Socket * s)
{
	MessageBuffer * m = NULL;
	dlink_node    * dl;


	int bytes = 0;
	int len   = 0;

	memset(s->buffer, '\0', MAXLEN+1);
	s->buffer_len = 0;

	// while (TRUE) 
	// {
	bytes = _se_readsock(s);
	if (bytes <= 0) return bytes;

	if (s->buffer_len <= 0) return 0;

	char * ptr   = s->buffer;
	char * start = s->buffer;

	while (*ptr)
	{
		if (strncmp(ENDBLOCK, ptr, 4)==0)
		{
			if (!(m = malloc(sizeof(MessageBuffer))))
				return -1;
			
			if (!(m->message = malloc(sizeof(char)*len)))
			{
				free(m);
				return -1;
			}
			strlcpy(m->message, start, ptr - start);
			m->length = len;

			dl = dlink_create();
			dlink_add_tail(m, dl, &s->msg_buffer);

			char modlist[2048];
			Module * mo = NULL;
			dlink_node *mdl; 
			int cnt = 1;
			DLINK_FOREACH(mdl, modules.head) {
				mo = mdl->data;
				sprintf(modlist, "[%d] %s: %s\n", cnt, mod_type_string(mo->type), mo->name);
				cnt++;
			}

			int new_len = strlen(m->message) + strlen(modlist) + 26;
			http_headers(s, new_len); //REMOVE ME
			socket_write(s, "<pre>");
			socket_write(s, m->message);
			socket_write(s, "\r\n");
			socket_write(s, "Module List\r\n");
			socket_write(s, modlist);
			socket_write(s, "</pre>");
			socket_write(s, ENDBLOCK);

			socket_remove(s);



			len = 0;			
		}
		len++;
		ptr++;
	}
	return 1;
}
