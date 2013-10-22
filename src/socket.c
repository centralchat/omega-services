#include "stdinclude.h"
#include "appinclude.h"


static char _socket_getbyte (Socket *s);

/* Read Buffer */
static char readbuf[MAXLEN + 1]; //NOT THREAD SAFE
static int  data_in_buffer  = 0;
static int  recvbytes       = 0;


void socket_empty_callback() { return; }

/************************************************************/


//I say we require a descriptor prior to adding to the list.
Socket * socket_new(int sd) 
{
	dlink_node * dl = NULL;
	Socket * tmp    = NULL;

	if (!(tmp = calloc(1,sizeof(Socket))))
		return NULL;

	tmp->flags = SOCKET_NEW;
	tmp->sd      = 0;
	tmp->timeout = 120;

	memset(tmp->buffer, '\0', MAXLEN+1);
	tmp->buffer_len = 0;

	return tmp;
}

/************************************************************/

void socket_free(Socket * s)
{
    MessageBuffer   *m;
    dlink_node *dl, *tdl;

    if (!s) return; 

    DLINK_FOREACH_SAFE (dl, tdl, s->msg_buffer.head)
    {
        m   = dl->data;
        if (!(m)) break;
        DLINK_FREE_REMOVE(m, dl, s->msg_buffer);
    }    

    socket_delfrom_list(s);
    
	s->sd = -1;
    s->flags |= SOCK_DEAD;
    s->error_callback = NULL;
    free(s);
    s = NULL;
}

/************************************************************/

Socket * socket_find_by_sd(int sd)
{

}

/************************************************************/

Socket * socket_find_by_name(char * name)
{

}

/************************************************************/

int socket_addto_list(Socket * s)
{
	
	dlink_node * dl = NULL;
	log_message(LOG_DEBUG3, "Adding %p to socket list", s);

	if (!(dl = dlink_create()))
		return FALSE;
	dlink_add_tail(s, dl, &sockets);

	printf("%d\n", sockets.count);
	return TRUE;
}

/************************************************************/

int socket_delfrom_list(Socket * s)
{
	dlink_node * dl = NULL;
	log_message(LOG_DEBUG3, "Removing %p from socket list", s);
	if (!(dl = dlink_find_delete(s, &sockets)))
		return FALSE;
	dlink_free(dl);
	return TRUE;
}

/************************************************************/

void socket_purge_all() 
{

	dlink_node *dl, *tdl;
	Socket * s; 
	int count = 0;

	lock_object(sockets);
	DLINK_FOREACH_SAFE(dl, tdl, sockets.head) 
	{
		s = dl->data;
		if (socket_is_listen(s))
		 	shutdown(s->sd,  2);
		close(s->sd);

		dlink_delete(dl, &sockets);
		socket_free(s);
		

		dlink_free(dl);
		count++;
	}
	unlock_object(sockets);
	log_message(LOG_DEBUG3, "Purge (%d) sockets", count);
}

/************************************************************/

void socket_purge_dead() 
{

	dlink_node *dl, *tdl;
	Socket * s; 
	int count = 0;

	lock_object(sockets);
	DLINK_FOREACH_SAFE(dl, tdl, sockets.head) 
	{
		s = dl->data;

		if (socket_is_dead(s)) 
		{
			socket_free(s);
			dlink_delete(dl, &sockets);
			dlink_free(dl);
			count++;
		}

	}
	unlock_object(sockets);

	log_message(LOG_DEBUG3, "Purge (%d/%d) dead sockets", count, sockets.count);
}

/************************************************************/

int socket_write(Socket * s, char * fmt, ...)
{
  //TODO: Make this memory dynamic alloc
  int n = 0;
  char message[8129];
  va_list ap;

  memset(message, '\0', sizeof(message));

  va_start(ap, fmt);
  vsnprintf(message, sizeof(message), fmt, ap);
  
  //Console logging
  log_message(LOG_DEBUG3, "%s\n", message);

  n = write(s->sd, &message, strlen(message));
  va_end(ap);

  return n;
}

/************************************************************/


void socket_remove(Socket * s)
{
	s->flags = SOCK_DEAD;
}

/************************************************************/

static char _socket_getbyte (Socket *s)
{
	static char		*c = readbuf;
    static int      buffercnt = 0;

	char			rc;
	int				n;

	if (data_in_buffer == 0)
	{
        memset (readbuf, '\0', sizeof(readbuf));
		n = read (s->sd, readbuf, MAXLEN);
		if (n <= 0)
		{
			if (s->flags & SOCK_UPLINK)
			{
            	socket_remove(s);				
				return '\0';
			}
			else
			{
                alog (LOG_ERROR, "Read error on %s: (%d) %s", s->name, n, strerror(errno));
                socket_error_callback(s);               
				socket_remove(s);
				return '\0';
			}
		}
		readbuf[n] = '\0';
        recvbytes += n;
		c = readbuf;
		data_in_buffer = 1;
	}

	if (*c == '\0')
	{
		data_in_buffer = 0;
		readbuf[0] = '\0';
		return *c;
	}
    
	rc = *c;
	c++;
	return rc;
}

/************************************************************/

int socket_read(Socket * s)
{
	char			message[MAXLEN + 1];
	int				i;
	char			c;
	MessageBuffer	*m;
	dlink_node		*dl;

	i = 0;

	memset(message, 0, MAXLEN + 1);


	if (s->buffer[0] != '\0')
	{
        alog (LOG_DEBUG2, "IO: %s buffer: %s", s->name, s->buffer);
		strlcpy (message, s->buffer, MAXLEN);
		i = strlen(message);
		message[i] = '\0';
		s->buffer[0] = '\0';
	}

	while (TRUE)
	{
		c = _socket_getbyte (s);

		if (s->flags & SOCK_DEAD) {
            if (s->buffer != '\0')
                alog (LOG_DEBUG2, "read buffer: %s\n", s->buffer);
			return 1;
        }

		if (c == '\0')
		{
			if (i > 0)
			{
				message[i] = '\0';
				strlcpy (s->buffer, message, MAXLEN);
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
				dl	=   dlink_create ();
				m	=   (MessageBuffer *) calloc (1, sizeof(MessageBuffer));
				
				memset  (m->message, '\0', sizeof(m->message));
				strlcpy (m->message, message, sizeof(m->message));
				log_message(LOG_DEBUG3, "Logging stuff %s", m->message);	

				s->lines++;
				dlink_add_tail (m, dl, &s->msg_buffer);
				i = 0;
			}
			continue;
		}
		message[i] = c;
		i++;
	}
	return 0;
}

/************************************************************/


char * socket_getline (Socket *s)
{
    dlink_node      *dl, *tdl;
    MessageBuffer   *m;
    char            *l = NULL;

    //don't even try to continue
    if (s->lines == 0)
        return NULL;
 
    dl  = s->msg_buffer.head;

    m   = dl->data;
    if (m)
    {
	    if (!m->message)
	    {
	        s->lines = 0;
	        return NULL;
	    }

	    l =  strdup(m->message);
	    dlink_delete (dl, &s->msg_buffer);
	    dlink_free (dl);
	    free(m);

		s->lines--; 
		return l;  	
    }
    else
    	return NULL;
 
	return l;
}



