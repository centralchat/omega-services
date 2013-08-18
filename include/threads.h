/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
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
 *    $Id: threads.h 2396 2012-06-30 16:53:15Z twitch $
 */

#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED


#define NUM_THREADS 100

#define THREAD_OK 0
#define THREAD_MUTEX_LOCK 1
#define THREAD_MUTEX_UNLOCK 2

typedef struct 
{


#ifdef HAVE_PTHREAD
   time_t spawned;

   pthread_t tid;

   pthread_mutex_t lock;

   pthread_cond_t cond;

   pthread_attr_t attr;

   void (*cleanup)(void *);

   void *params;

   uint32 flags;

   int working;
#endif

} Thread;


dlink_list threadlist;

void dest_all_threads	(void);

Thread* spawn_thread(void* params, void (*init_fnct)(void *),void (*exit_fnct)(void *));


Thread* find_thread	  ();
int     dest_thread	  ();

int mutex_state			(Thread* ,int);
int thread_wait			(Thread*, pthread_cond_t*);
int thread_signal		(pthread_cond_t*);
int thread_cancle		(Thread*);
int thread_count;




#endif // THREADS_H_INCLUDED
