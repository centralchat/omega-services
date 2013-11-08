#include "stdinclude.h"
#include "appinclude.h"


/************************************************************/


command_t * command_new(char * name, int (*handler)(args_t *))
{
	command_t * c = NULL;

	if (!(c = malloc(sizeof(command_t))))
		return NULL;

	c->event_string = strdup(name);
	c->handler      = handler;

	return c;
}

/************************************************************/

void command_free(command_t * c)
{
	if (!c)
		return;

	free(c);
}


/************************************************************/


int  command_add  (dlink_list *list, char *name, int (*handler)(args_t *)) 
{
	dlink_node *tdl, *dl;
	dlink_list *l = &list[COMMAND_HASH(name)];
	command_t  *c = NULL;

	if (!l)
		return FALSE;

	thread_lock_obj(&l, THREAD_MUTEX_FORCE_LOCK);

	if (!(c = command_new(name, handler)))
		return FALSE;

	dl = dlink_create();
	dlink_add_tail(c, dl, l);

	thread_lock_obj(&l, THREAD_MUTEX_UNLOCK);	

	return TRUE;
}

/************************************************************/

void command_del  (dlink_list * list, char *name, int (*handler)(args_t *))
{
	dlink_node *tdl, *dl;
	dlink_list *l = &list[COMMAND_HASH(name)];
	command_t  *c = NULL;

	if (!l)
		return;

	thread_lock_obj(&l, THREAD_MUTEX_FORCE_LOCK);
	DLINK_FOREACH_SAFE(dl, tdl, l->head)
	{
		c = dl->data;
		if ((!strcasecmp(c->event_string, name)) && (c->handler = handler))
		{
			dlink_delete(dl, l);
			dlink_free(dl);
			command_free(c);
		}
	}

	thread_lock_obj(&l, THREAD_MUTEX_UNLOCK);	
	return;
}

/************************************************************/

int  command_emit (dlink_list *list, char *cmd, args_t *args)
{

	dlink_node *tdl, *dl;
	dlink_list *l = &list[COMMAND_HASH(cmd)];
	command_t  *c = NULL;

	int res  = 0;

	if (!l)
		return FALSE;

	thread_lock_obj(&l, THREAD_MUTEX_FORCE_LOCK);

	log_message(LOG_EVENT, "Running command %s", cmd);
	DLINK_FOREACH_SAFE(dl, tdl, l->head)
	{
		c = dl->data;
		if ((!strcasecmp(c->event_string, cmd)))
		{
			if (!(res = c->handler(args)))
			{
				log_message(LOG_EVENT, "Command chain stopped for event %s", cmd);
				break;
			}
		}
	}
	thread_lock_obj(&l, THREAD_MUTEX_UNLOCK);	
	return res;
}

/************************************************************/
