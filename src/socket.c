#include "stdinclude.h"
#include "appinclude.h"


void socket_empty_callback() { return; }

//I say we require a descriptor prior to adding to the list.
Socket * socket_new(int sd) 
{
	dlink_node * dl = NULL;
	Socket * tmp    = NULL;

	if (!(tmp = malloc(sizeof(Socket))))
		return NULL;

	tmp->flags = SOCKET_NEW;
	tmp->sd      = 0;
	tmp->timeout = 120;

	memset(tmp->buffer, '\0', MAXLEN+1);
	tmp->buffer_len = 0;

	return tmp;
}


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
}

Socket * socket_find_by_sd(int sd)
{

}


Socket * socket_find_by_name(char * name)
{

}


int socket_addto_list(Socket * s)
{
	dlink_node * dl = NULL;
	if (!(dl = dlink_create()))
		return FALSE;
	dlink_add(s, dl, &sockets);
	return TRUE;
}

int socket_delfrom_list(Socket * s)
{
	dlink_node * dl = NULL;
	DLINK_REMOVE(s, dl, sockets);
	// if (!(dl = dlink_find_delete(s, &sockets)))
	// 	return FALSE;
	// dlink_free(dl);
	return TRUE;
}

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

		socket_free(s);
		count++;
	}
	unlock_object(sockets);
	log_message(LOG_DEBUG3, "Purge (%d) sockets", count);
}

void socket_purge_dead() 
{

	dlink_node *dl, *tdl;
	Socket * s; 
	int count = 0;

	lock_object(sockets);
	DLINK_FOREACH_SAFE(dl, tdl, sockets.head) 
	{
		s = dl->data;
		if (socket_is_dead(s)) {
			socket_free(s);
			count++;
		}

	}
	unlock_object(sockets);
	log_message(LOG_DEBUG3, "Purge (%d) dead sockets", count);
}


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


void socket_remove(Socket * s)
{
	s->flags = SOCK_DEAD;
}