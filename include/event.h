#ifndef __EVENTHANDLER_H__
#define __EVENTHANDLER_H__


//events are timed.
#define EVENT_TYPE_TIMED      0
#define EVENT_TYPE_EMIT       1

typedef enum { 
	EVENT_SUCCESS = 0,
	EVENT_HALT,
	EVENT_ERROR,
	EVENT_FATAL,
	EVENT_SKIP,
	EVENT_RUN_LATER
} event_return_t;

typedef struct { 
	INFO_ATTRIBUTES;

	int priority;
	event_return_t (*handler)(args_t *);
} event_t;

typedef struct {
	char name[32];
	int  runtime;
	int  lastran;
	int  type;	
	dlink_list events;
} event_chain_t;


dlink_list eventlist;

#define event(evt,arg)                    _event_dispatch(_A_, evt, arg)
#define event_dispatch(evt, arg)          _event_dispatch(_A_, evt, arg)

#define event_add(evt, fnct)              _event_add(_A_, evt, EVENT_TYPE_EMIT, fnct)
#define event_add_timed(evt, fnct)        _event_add(_A_, evt, EVENT_TYPE_EMIT, fnct)
#define event_remove(evt, fnct, type)     _event_remove(_A_, evt, type, fnct)

void event_init         (void);

int _event_dispatch     (char *, int, const char *, const char *, args_t *);
int _event_add          (char *, int, const char *, const char *, int, event_return_t (*)(args_t *));
int _event_remove       (char *, int, const char *, const char *, int, event_return_t (*)(args_t *));

//Internal mechanism of fireing an event. Called by both timed and dispatch event
//controllers.
int event_fire          (event_t *, args_t *);

event_chain_t * event_chain_find_or_new(const char *, int);
event_chain_t * event_chain_new        (const char *, int);
event_chain_t * event_chain_find       (const char *, int);

#endif //__EVENTHANDLER_H__

