#include "stdinclude.h"
#include "appinclude.h"

/*******************************************************************/

static const char * event_type_string(int type)
{
	switch (type)
	{
		case EVENT_TYPE_TIMED:
			return "Timed";
		case EVENT_TYPE_EMIT:
			return "Emittable";
	}
	return "";
}

/*****************************************************************/

void event_init()
{
	eventlist.head = NULL;
	eventlist.tail = NULL;	
}


/*****************************************************************/

int _event_dispatch 
(char * file, int line, char * func, char * name, event_args_t * args)
{
	
	event_chain_t * ec = NULL;
	event_t       * e  = NULL;
	dlink_node    * dl = NULL, 
	              *tdl = NULL;
	event_return_t ret;
	int           halt = FALSE;

	if (!(ec = event_chain_find(name, EVENT_TYPE_EMIT)))
	{
		log_message(LOG_DEBUG, "(%s:%d) No events in chain %s", file, line, name);
		return TRUE;
	}

	log_message(LOG_EVENT, "(%s:%d) Dispatching event %s", file, line, name);

	thread_lock_obj(&ec->events, THREAD_MUTEX_FORCE_LOCK);
	DLINK_FOREACH_SAFE(dl, tdl, ec->events.head)
	{
		e = dl->data;
		ret = e->handler(args);
		switch (ret)
		{
			case EVENT_SUCCESS:
			case EVENT_SKIP:
			case EVENT_RUN_LATER:
				halt = FALSE;
				break;
			case EVENT_HALT:
			case EVENT_ERROR:
			case EVENT_FATAL:
				log_message(LOG_DEBUG, "Filter chain halted");
				halt = TRUE;
				break;
		}
		if (halt) break;

	}
	thread_lock_obj(&ec->events, THREAD_MUTEX_UNLOCK);

	return TRUE;
}

/*****************************************************************/

int _event_dispatch_timed()
{


}



/*****************************************************************/
//__FILE__, __LINE__, __PRETTY_FUNCTION__
int _event_add
(char *file, int line, char *func, char *name, int type, 
	int prio, event_return_t (*handler)(event_args_t *))
{
	dlink_node    * dl  = NULL;
	event_chain_t * ec  = NULL;
	event_t       * evt = NULL;

	if (!(ec = event_chain_find_or_new(name, type)))
		return FALSE;

	if (!(evt = calloc(1, sizeof(event_t))))
		return FALSE;

	log_message(LOG_EVENT, "Adding event to chain: %s type: %s", 
		name, event_type_string(type));

	strlcpy(evt->file, file, sizeof(evt->file));
	strlcpy(evt->func, func, sizeof(evt->func));

	evt->lineno   = line;
	evt->handler  = handler;
	evt->priority = prio;

	dl = dlink_create();
	dlink_add_tail(evt, dl, &ec->events);

	return TRUE;
}

/*****************************************************************/

int _event_remove 
(char *file, int line, char *fnct, char *name, int type, 
	int prio, event_return_t (*handr)(event_args_t *))
{

}

/*****************************************************************/

event_chain_t * event_chain_find_or_new(char * name, int type) 
{
	event_chain_t * ec = NULL;
	if (!(ec = event_chain_find(name, type))) {
		log_message(LOG_EVENT, "Event chain is empty creating event: %s", 
			name, event_type_string(type));
		ec = event_chain_new(name, type);
	}
	return ec;
}

/*****************************************************************/

event_chain_t * event_chain_new(char * name, int type) 
{
	dlink_node    * dl  = NULL;				 
	event_chain_t * ec  = NULL;

	if (!(ec = calloc(1, sizeof(event_chain_t))))
		return NULL;

	//not nesecary since we calloc
	ec->events.head = NULL;
	ec->events.tail = NULL;


	strlcpy(ec->name, ec->name, sizeof(ec->name));
	ec->type = type;

	return ec;
}

/*****************************************************************/

event_chain_t * event_chain_find(char * name, int type) 
{
	dlink_node    * dl  = NULL,
				  * tdl = NULL;				 
	event_chain_t * ec  = NULL;

	DLINK_FOREACH_SAFE(dl,tdl, eventlist.head)
	{
		ec = dl->data;
		if (ec->type != type)
			continue;

		if (!strcasecmp(ec->name, name))
			break;
		else
			ec = NULL;
	}
	return ec;
}

/*******************************************************************/

