/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
 * 
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 *
 *    $Id: threads.c 2396 2012-06-30 16:53:15Z twitch $
 */

#include "stdinc.h"
#include "server.h"

/**
 * has_dest_threads - Is only true if the core has tried to
 * destroy all threads previously for what ever reason. This 
 * prevents from dest_all_threads to be called more then once
 * as it can be possibly fatal.      
 */ 

int has_dest_threads = 0;

pthread_mutex_t    exit_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * spawn_thread  - spawns a thread and sets it to the detached state. This thread
 * I accessible by accessing the structure which is returned upon a successful call.
 * these threads are globally usable and do not have any predefined behavior, besides
 * being detached on creation. 
 * 
 * 
 * @param params The arguments to be passed to both init_fnct and exit_fnct
 * @param init_fnct A function pointer to the entry point of the spawning thread
 *                  threads jump into this at the beginning of normal execution.
 * @param exit_fnct This is a pointer to the cleanup function, which is called before
 *                  a thread destroys.
 * @return Thread* upon successful call and null upon failure.
 *
 *     - Code segments that use threading are required to store the Thread* pointer returned
 *       as only threads them selves can look it up via its TID. 
 */

 
Thread* spawn_thread(void *params, void (*init_fnct)(void *),void (*exit_fnct)(void *))
{

#ifdef    HAVE_PTHREAD

    Thread *t;
    dlink_node *dl;

    int status = 0;
    int thread_current = 0;
	struct sched_param param;

    if (!thread_count)
        thread_count = 0;

    t = (Thread*) malloc(sizeof(Thread));

    pthread_mutex_init(&t->lock, NULL); //initialize the mutex

    pthread_attr_init(&t->attr);
    pthread_attr_setdetachstate(&t->attr, PTHREAD_CREATE_DETACHED);

    pthread_cond_init (&t->cond, NULL);
	pthread_attr_setschedparam (&t->attr, &param);


    status = pthread_create(&t->tid, &t->attr, (void*) init_fnct, params);

    switch (status)
    {
                case 0:
                    thread_count++; //only increment our thread_count if we successfully spawn :)
                    break;

                case EAGAIN:
						free(t);
						alog(LOG_ERROR,"Unable to spawn threaded process - necessary resources locked");
                        return NULL;
                case EINVAL:
						free(t);
                        alog(LOG_ERROR,"Unable to spawn threaded process - invalid attributes");
                        return NULL;
                case EPERM:
						free(t);
                        alog(LOG_ERROR,"Unable to spawn threaded process - incorrect permissions");
                        return NULL;
    }

	if (params)
		t->params = params;
	else
		t->params = NULL;
		
	if (exit_fnct)
        t->cleanup = exit_fnct;
		
    dl = dlink_create();
    dlink_add_tail(t,dl,&threadlist);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
    //detach from the thread.
    pthread_detach(t->tid);

    return t;

#endif /* HAVE_PTHREAD */

    return NULL;
}

/**
 * mutex_state - This function locks the thread mutex found in Thread::mutex
 *               it also prevents deadlock from occurring within the threading
 *               by implementing pthread_trylock().
 * @param thread - A pointer to the thread structure holding our threads data. 
 * @param  lock - weather or not we are to lock our mutex or unlock it 
 *  
 * @return true/false True if lock/unlock is successful false on failure.
 */
 
int mutex_state(Thread* thread, int lock)
{


    s_assert(&thread != NULL);
    s_assert(lock >= 0);

    if (!thread)
		return 0;

#ifdef HAVE_PTHREAD

       switch (lock)
       {

            case THREAD_MUTEX_LOCK:
                    return pthread_mutex_trylock(&thread->lock);
            case THREAD_MUTEX_UNLOCK:
                    return pthread_mutex_unlock(&thread->lock);

        }

#endif

    return -1;
}

/**
 * find_thread - Find a thread by corresponding TID 
 * note only threads can call this function as it uses
 * pthread_self to determine the thread we are looking
 * for.
 * 
 * @return
 *     Thread* on successful find of corresponding thread structure
 *     NULL on a failure to find the thread. 
 *
 */
 

Thread *find_thread()
{
#ifdef HAVE_PTHREAD

	dlink_node *dl,*tdl;
	Thread *thread;

	DLINK_FOREACH_SAFE(dl,tdl,threadlist.head)
	{
		thread = dl->data;

		if (pthread_equal(pthread_self(), thread->tid))
			return thread;
	}
#endif
	return NULL;

}

/**
 * dest_thread - Destroy calling thread and remove it from threadlist
 * all threads are required to call this function before exiting as it
 * insures cleanup and prevents stale thread entries.
 *
 * @return true/false - This function returns true if the thread
 *                      exists and can be destroyed. and false otherwise.
 */
 

int dest_thread()
{

#ifdef HAVE_PTHREAD
	dlink_node *dl,*tdl;
	Thread *thread;

        /**
         * Wonder if this is where the issues are coming from, the 
         * thread could very well be stuck in EBUSY
         */

	while (pthread_mutex_trylock(&exit_mutex)==EBUSY)
		sleep(1);
					
	DLINK_FOREACH_SAFE(dl,tdl,threadlist.head)
	{
		thread = dl->data;

		if (!thread)
			goto destend; //something is seriously effed bail.
			
		if (pthread_equal(pthread_self(), thread->tid))
		{
			
			dl = dlink_find_delete(thread,&threadlist);
			dlink_free(dl);			
			break;
		}
		continue;
	}
	
	
	if (thread_count > 0)
		thread_count--;
	
	pthread_mutex_destroy(&thread->lock);
	pthread_attr_destroy(&thread->attr);
	
	free(thread);
	
	pthread_mutex_unlock(&exit_mutex);
	
	pthread_testcancel();
	
	pthread_exit((void*) NULL);
	
#endif
	return 1;
destend:
	return 0;
}

/**
 * dest_all_threads - This destroys all threads within
 * the thread list, and removes their entries from the list. 
 *
 * 
 * @return void - No return see logs if errors occur
 * 
 * @note This should only be called upon exit as it has the
 *       side affect of destroying socketengine threads as it
 *       works on an unbias. 
 */

void dest_all_threads(void)
{

#ifdef HAVE_PTHREAD

	dlink_node *dl,*tdl;
	Thread *thread;
	int thread_total = 0;
	int thread_status = 0;

	if (!has_dest_threads)
        has_dest_threads = 1;
    else
        return;

	DLINK_FOREACH_SAFE(dl,tdl,threadlist.head)
	{
		thread = dl->data;	
		
		if (thread->cleanup)
					thread->cleanup(thread->params);
		
		thread_status = pthread_cancel(thread->tid);
		
		pthread_mutex_destroy(&thread->lock);
		pthread_attr_destroy(&thread->attr);
		
		switch (thread_status)
		{
			case 0:
				break;
			case ESRCH:
				break;
			default:
				alog(LOG_ERROR,"Thread Error, Unable to destroy thread unknown return in pthread_cancel");
				break;
		}

		if (thread_count > 0)
			thread_count--;

		dl = dlink_find_delete(thread,&threadlist);
		dlink_free(dl);
		
		if (thread)
			free(thread);

	}

	alog(2,"Destroyed %d thread(s)",thread_count);
#endif

	return;
}


int thread_cancle(Thread *thread)
{

#ifdef HAVE_PTHREAD
		dlink_node *dl;
		int thread_total = 0;
		int status = 0;
		
		if ((!thread)  || (thread == NULL))
				return 0;
			
		if (thread->cleanup)
					thread->cleanup(thread->params);

		switch ((status = pthread_cancel(thread->tid)))
		{
			case 0:
				break;
			case ESRCH:
				break;
			default:
				alog(LOG_ERROR, "Thread Error, Unable to destroy thread unknown return in pthread_cancel [%d]", status);
				break;
		}

		if (thread_count > 0)
			thread_count--;

		dl = dlink_find_delete(thread,&threadlist);
		dlink_free(dl);
		
		if (thread)
			free(thread);
#endif
		return 1;
}


int thread_wait(Thread* t, pthread_cond_t *cond)
{
#ifdef HAVE_PTHREAD
	return pthread_cond_wait(cond, &t->lock);
#endif
}

int thread_signal(pthread_cond_t *cond)
{
#ifdef HAVE_PTHREAD
	return pthread_cond_signal(cond);
#endif
}


