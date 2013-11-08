#include "stdinclude.h"
#include "appinclude.h"

/************************************************************/

server_t * server_new()
{
	server_t * tmp = NULL;
	if (!(tmp = calloc(1, sizeof(server_t))))
		return NULL;

	return tmp;
}

/************************************************************/

server_t * server_init(char * name, char * desc, char * uid)
{
	server_t   * tmp = NULL;
	dlink_node * dl  = NULL;
	if (!(tmp = server_new()))
		return NULL;

	strncpy(tmp->name, name, sizeof(tmp->name));
	strncpy(tmp->uid,   uid, sizeof(tmp->uid));
	strncpy(tmp->desc, desc, sizeof(tmp->desc));


	return tmp;
}


/************************************************************/

void server_addto_list(server_t * srv)
{
	dlink_node * dl;

	thread_lock_obj(&servers, THREAD_MUTEX_FORCE_LOCK);

	dl = dlink_create();
	dlink_add_tail(srv, dl, &servers);

	thread_lock_obj(&servers, THREAD_MUTEX_UNLOCK);
}

/************************************************************/

void server_delfrom_list(server_t * srv)
{
	dlink_node * dl;

	thread_lock_obj(&servers, THREAD_MUTEX_FORCE_LOCK);
	
	dl = dlink_find_delete(srv, &servers);
	dlink_free(dl);

	thread_lock_obj(&servers, THREAD_MUTEX_UNLOCK);

}

/************************************************************/

void server_free(server_t * srv)
{
	if (srv)
		free(srv);
}

/************************************************************/

server_t * server_findby_name (char * name) 
{
	server_t * srv;
	dlink_node *dl;
	DLINK_FOREACH(dl, servers.head)
	{
		srv = dl->data;
		if (!strcasecmp(srv->name, name))
			return srv;
	}
	return NULL;
}

/************************************************************/

server_t * server_findby_uid  (char * uid)
{
	server_t * srv;
	dlink_node *dl;
	DLINK_FOREACH(dl, servers.head)
	{
		srv = dl->data;
		if (!strcasecmp(srv->uid, uid))
			return srv;
	}
	return NULL;
}

/************************************************************/
