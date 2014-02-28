#include "stdinclude.h"
#include "appinclude.h"


/************************************************************/
/**
 *      Module Que Operations
 */
 

modque_t * find_mod_que(char *file)
{
  dlink_node *dl;
  modque_t *mq;
  
  DLINK_FOREACH(dl,moduleque.head)
  {
      mq = dl->data;
      if (strcasecmp(mq->name,file)==0)
          return mq;
  }
  return NULL;
 }

 
 /************************************************************/



int addto_mod_que_ext(char *file,int type, int act, int order)
{

  dlink_node *dl;
  modque_t *mq;

  if (!(mq = (modque_t*) malloc(sizeof(modque_t))))
    return MOD_ERR_MEMORY;
  
  
  strlcpy(mq->name,file,sizeof(mq->name));
  
  mq->type   = type;
  mq->action = act;
  mq->load   = order;
  
  dl = dlink_create();
  dlink_add_tail(mq,dl,&moduleque);
  return MOD_ERR_OK;
}

/************************************************************/

int run_mod_que(int load)
{
  dlink_node *dl,*tdl;
  modque_t *mq;
  int status;
  
  // Their is nothing to run
  if (moduleque.count == 0)
    return 1;

  // thread_lock_obj(&moduleque, THREAD_MUTEX_FORCE_LOCK);
  log_message(LOG_DEBUG3, "Running module queue [%s]", mod_queue_prio_string(load));
  DLINK_FOREACH_SAFE(dl,tdl,moduleque.head)
  {
    mq = dl->data;
    
    if (!mq->name)
      continue;
      
    switch (mq->action)
    {
      case MOD_ACT_LOAD:
        if ((load == -1) || (mq->load != load) || (load == MOD_QUEUE_PRIO_NONE))
            goto cont;
            
        if ((module_find(mq->name)))
            break;
          
        status = module_open(mq->name, mq->type);

        log_message(LOG_MODULE,"Loading module %s [%s]",mq->name,GetModErr(status));

        // We have an unmet dependency, try to load it next time around. 
        // if we are on MOD_QUEUE_PRIO_POST then fail it completely.        
        if ((mq->load != MOD_QUEUE_PRIO_POST) && (status == MOD_ERR_DEPENDENCY)) {
          mq->load += 1;
          goto cont;
        }
        
        break;
      case MOD_ACT_UNLOAD:
        status = module_close(mq->name);
        alog(LOG_MODULE,"Unloading module %s [%s]",mq->name,GetModErr(status));
        break;
      case MOD_ACT_RELOAD:
        if ((status = module_close(mq->name)) == 0)
          status = module_open(mq->name, mq->type);
              
        alog(LOG_MODULE,"Reloading module %s [%s]",mq->name,GetModErr(status));
        break;
    }
freedl:
    dlink_delete(dl,&moduleque);
    dlink_free(dl);
      
    if (mq)
      free(mq);
cont:
    continue;
  }

  // thread_lock_obj(&moduleque, THREAD_MUTEX_UNLOCK); 

  return 1;
}

/************************************************************/

