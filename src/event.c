#include "modinclude.h"
#include "stdinclude.h"
#include "appinclude.h"

/*******************************************************************/

static const char * event_type_string(int type)
{
	const char * type_strings[] = {
		"Timed",
		"Emittable"
	};
	return type_strings[type];
}

/*****************************************************************/

void event_init()
{
	eventlist.head = NULL;
	eventlist.tail = NULL;	
}


/*****************************************************************/

int _event_dispatch 
(char * file, int line, const char * func, const char * name, args_t * args)
{
	
	event_return_t ret;
	event_chain_t * ec  = NULL;
	event_t       * e   = NULL;
	dlink_node    * dl  = NULL, 
	              * tdl = NULL;

	int           halt  = FALSE;
	int           cnt   = 0;

	if (!(ec = event_chain_find(name, EVENT_TYPE_EMIT)))
	{
		log_message(LOG_DEBUG, "(%s:%d) No events in chain %s", file, line, name);
		return TRUE;
	}

	log_message(LOG_EVENT, "(%s:%d) Dispatching event %s", file, line, name);

	// thread_lock_obj(&ec->events, THREAD_MUTEX_FORCE_LOCK);
	DLINK_FOREACH_SAFE(dl, tdl, ec->events.head)
	{
		printf("looping\n");
		e = dl->data;
		cnt++;
		if (!event_fire(e, NULL)) 
		{
			log_message(LOG_EVENT, "Event chain halted");
			break;
		}
	}
	//thread_lock_obj(&ec->events, THREAD_MUTEX_UNLOCK);

	log_message(LOG_DEBUG, "Ran %d events in chain", cnt);

	return TRUE;
}

/*****************************************************************/

int _event_dispatch_timed()
{

	dlink_node    * dl  = NULL,
				        * tdl = NULL;

	event_chain_t * ec  = NULL;
	event_t       *  e  = NULL;


	int            ret  = 0,
	               cnt  = 0,
	              halt  = 0;

	thread_lock_obj(&eventlist, THREAD_MUTEX_FORCE_LOCK);

	DLINK_FOREACH_SAFE(dl,tdl, eventlist.head)
	{
		cnt = 0;
		ec  = dl->data;

		if (ec->type != EVENT_TYPE_TIMED)
			continue;

		//Do we need to run again?
		if ((ec->lastran + ec->runtime) > time(NULL)) 
			continue;

		ec->lastran = time(NULL); //update lastran time

		DLINK_FOREACH_SAFE(dl, tdl, ec->events.head)
		{
			e = dl->data;
			cnt++;

			if (!event_fire(e, NULL)) 
			{
				log_message(LOG_EVENT, "Timed event chain halted");
				break;
			}
		}

		log_message(LOG_EVENT, "Ran %d events in chain %s", cnt, ec->name);
	}
	thread_lock_obj(&eventlist, THREAD_MUTEX_UNLOCK);

	return 0;
}

/*****************************************************************/

int event_fire(event_t * e, args_t * args)
{
		int ret = e->handler(args);
		switch (ret)
		{
			case EVENT_SUCCESS:
			case EVENT_SKIP:
			case EVENT_RUN_LATER:
				return TRUE;
			case EVENT_HALT:
			case EVENT_ERROR:
			case EVENT_FATAL:
				log_message(LOG_EVENT, "(%s:%d/%s) Event returned non successful value (%d)", e->file, e->lineno, e->func, ret);
				return FALSE;
		}
		return FALSE;
}


/*****************************************************************/
//__FILE__, __LINE__, __PRETTY_FUNCTION__
int _event_add
(char *file, int line, const char *func, const char *name, int type, 
	event_return_t (*handler)(args_t *))
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
	evt->priority = 0;

	dl = dlink_create();
	dlink_add_tail(evt, dl, &ec->events);

	return TRUE;
}

/*****************************************************************/

int _event_remove 
(char *file, int line, const char *fnct, const char *name, int type, 
	event_return_t (*handr)(args_t *))
{
	return 0;
}

/*****************************************************************/

event_chain_t * event_chain_find_or_new(const char * name, int type) 
{
	event_chain_t * ec = NULL;
	if (!(ec = event_chain_find(name, type))) {
		log_message(LOG_EVENT, "Event chain is empty creating event: %s type: %s", 
			name, event_type_string(type));
		ec = event_chain_new(name, type);		
	}
	return ec;
}

/*****************************************************************/

event_chain_t * event_chain_new(const char * name, int type) 
{
	dlink_node    * dl  = NULL;				 
	event_chain_t * ec  = NULL;

	if (!(ec = calloc(1, sizeof(event_chain_t))))
		return NULL;

	//not nesecary since we calloc
	ec->events.head = NULL;
	ec->events.tail = NULL;

	strlcpy(ec->name, name, sizeof(ec->name));

	ec->lastran = 0;
	ec->runtime = 0;
	ec->type    = type;

	dl = dlink_create();
	dlink_add_tail(ec, dl, &eventlist);

	log_message(LOG_DEBUG3, "Creating event chain for %s", name);
	return ec;
}

/*****************************************************************/

event_chain_t * event_chain_find(const char * name, int type) 
{
	dlink_node    * dl  = NULL,
				        * tdl = NULL;				 
	event_chain_t * ec  = NULL;

	DLINK_FOREACH_SAFE(dl,tdl, eventlist.head)
	{
		ec = dl->data;
		if (ec->type != type)
			continue;

		if (strcasecmp(ec->name, name)==0)
			break;
		else
			ec = NULL;
	}
	return ec;
}

/*******************************************************************/

