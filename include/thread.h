#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED


#define NUM_THREADS 100

#define THREAD_OK 0


#define THREAD_MUTEX_FORCE_LOCK 1
#define THREAD_MUTEX_LOCK       2
#define THREAD_MUTEX_UNLOCK     3

// Keep shit from breaking in non pthread 
#ifndef HAVE_PTHREAD
#  define pthread_mutex_t void
#  define pthread_attr_t  void 
#  define pthread_cond_t  void
#endif 


typedef struct 
{
	#ifdef HAVE_PTHREAD
		pthread_mutex_t lock;
		pthread_cond_t  cond;
	#endif

   time_t locked_at;
   time_t expires; // Do this so we can reap mutexes 
   					// good for databases
} thread_mutex_t;



typedef struct 
{
   time_t spawned;

   void (*cleanup)(void *);

   void *params;

   uint32 flags;

   int working;

#ifdef HAVE_PTHREAD
   pthread_attr_t attr;

   pthread_t tid;
   
   thread_mutex_t mutex;

#endif

} Thread;

void dest_all_threads	(void);

Thread* thread_spawn(void *, void (*init_fnct)(void *),void (*exit_fnct)(void *));

Thread* thread_find	           ();
int     thread_self_destruct	  ();


int thread_lock_obj              (void *, int);
int thread_set_mutex_state			(Thread * ,int);
int thread_wait			         (Thread *);
int thread_signal		            (Thread *);
int thread_cond_signal           (pthread_cond_t *);
int thread_wait_for_cond         (Thread* t, pthread_cond_t *);
int thread_cancel		            (Thread*);


#define thread_lock_mutex(t,opt) thread_lock_obj(&(x)->mutex.lock, opt) 
#define thread_lock_mutex(t,opt) thread_lock_obj(&(x)->mutex.lock, opt) 
#define lock_object(x)   thread_lock_obj(&(&(x))->lock, THREAD_MUTEX_FORCE_LOCK)
#define lock_object_ptr(x)   thread_lock_obj(&(x)->lock, THREAD_MUTEX_FORCE_LOCK)
#define unlock_object(x) thread_lock_obj(&(&(x))->lock, THREAD_MUTEX_UNLOCK)
#define unlock_object_ptr(x) thread_lock_obj(&((x))->lock, THREAD_MUTEX_UNLOCK)


dlink_list threadlist;


#endif // THREADS_H_INCLUDED
