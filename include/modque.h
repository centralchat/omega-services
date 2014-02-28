#ifndef __MODQUE_H__
#define __MODQUE_H__



/************************************************************/
/**
 * Module Que Structures
 */


typedef struct moduleq_entry {
  char name[256];
  int action;
  int load;
  int type;

} modque_t;


dlink_list moduleque;


/************************************************************/
/**
 * Module Que Functions
 */

modque_t * find_mod_que(char *);
int run_mod_que(int);

int addto_mod_que_ext(char *, int, int, int);

//Api to wrap the module que in a sensible mannor
#define addto_mod_que(name, act, ord) \
  addto_mod_que_ext((name), MOD_TYPE_UNKNOWN, (act), (ord))  
#define mq_preload_module(name, type) \
  addto_mod_que_ext(name, type, MOD_ACT_LOAD, MOD_QUEUE_PRIO_PRE)  
#define mq_load_module(name, type) \
  addto_mod_que_ext(name, type, MOD_ACT_LOAD, MOD_QUEUE_PRIO_STD)
#define mq_reload_module(name, type) \
  addto_mod_que_ext(name, type, MOD_ACT_RELOAD, MOD_QUEUE_PRIO_STD)
#define mq_reload_unmodule(name) \
  addto_mod_que_ext(name, MOD_TYPE_UNKNOWN, MOD_ACT_UNLOAD, MOD_QUEUE_PRIO_STD)




#endif //__MODQUE_H__