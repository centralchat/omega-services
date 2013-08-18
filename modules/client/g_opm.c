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
 *    $Id: g_opm.c 2394 2012-06-30 15:56:22Z twitch $
 */


#include "stdinc.h"
#include "server.h"
#include "opm.h"
#include "opm_error.h"
#include "opm_types.h"
#include "compat.h"

#ifdef inet_pton
#undef inet_pton
#endif 

#include <arpa/inet.h>

#define NAME "g_opm"
#define AUTHOR "Omega Team"
#define VERSION "$Id: g_opm.c 2394 2012-06-30 15:56:22Z twitch $"

#define ARRAY_SIZEOF(x) (sizeof(x) / sizeof((x)[0]))

#if 0
	OPM_T *scanner;
#endif

#ifdef HAVE_PTHREAD
pthread_t complete = 0;
#endif 

void opm_entry(void *);
void opm_exit (void *);


void open_proxy(OPM_T *, OPM_REMOTE_T *, int, void *);
void negotiation_failed(OPM_T *, OPM_REMOTE_T *, int, void *);
void opm_timeout(OPM_T *, OPM_REMOTE_T *, int, void *);
void end(OPM_T *, OPM_REMOTE_T *, int, void *);
void handle_error(OPM_T *, OPM_REMOTE_T *, int, void *);

static int event_opmcon(int, void *);

int opm_state = 1;

int opm_scan_count = 0;
int opm_thread_bail = 0;


dlink_list opm_scan_que;

#ifdef HAVE_PTHREAD
pthread_cond_t opm_cond;
#endif 

Thread *opm_thread;


unsigned short http_ports[] = { 8000, 8080, 3128, 80 };
unsigned short wingate_ports[] = { 23 };
unsigned short router_ports[] = { 23 };
unsigned short socks4_ports[] = { 1080 };
unsigned short socks5_ports[] = { 1080 };
unsigned short httppost_ports[] = { 80, 8090, 3128 };

int fdlimit = 1024;
int max_read = 4096;
int port = 6667;
int err_code = 0;
int opm_thread_wait = 0;

int gopm_load();
void gopm_unload(); 

static void * gopm_signals(void * args);

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4), gopm_load, gopm_unload);


int gopm_load()
{

	opm_state = 1;
	
	int status, ch;
	//if we don't have pthreads bail before we do anything. 
	#ifndef HAVE_PTHREAD
		throwModErr("Module Error, Threading is disabled or not available.","");
	#endif
	
	if (!(opm_thread = spawn_thread(NULL, opm_entry, NULL)))
		return MOD_STOP;
			
	AddEvent ("USERCONN", event_opmcon);
	return MOD_CONT;
	
}




void gopm_unload()
{
	opm_state = 0;
	
	DelEvent("USERCON",event_opmcon);
	return;
}




static int event_opmcon(int ac, void *user)
{
	User *u = user;
	unsigned char addr[16];
	dlink_node *dl, *idl;
	char *ip;
	
	if (sync_state != RUNNING)
		return;

	if (u->service)
		return;

	inet_pton(AF_INET,u->ip,&addr);

	if ((match_cidr(addr, "\x7F", 8)) || (match_cidr(addr, "\0", 8)) || match_cidr(addr,"10.0.0.0", 8) ||
            match_cidr(addr,"172.16.0.0", 12)) 
			return;
		
    if (strcasecmp(u->ip, CfgSettings.local_ip)==0)
			return;
			
	/*
	 *protect against our user leaving, we should not let this be touched, course we 
	 *could block, however could cause issues - so lose.
	 */
	
	
	if (!u->ip)
			return;
	if (!(ip = (char*) malloc(sizeof(u->ip)+1)))
			return;

#ifdef HAVE_PTHREADS
	
	memcpy (ip, u->ip, sizeof(u->ip));		
	sendto_one_notice(s_Guardian, u, "Your connection is currently being scanned for open proxies.");		
	/** Thread is busy, add it to the queue we can scan off later. **/
	while ((mutex_state(opm_thread, THREAD_MUTEX_LOCK)==EBUSY) && (!opm_thread_wait))
		return; //return here for now
		
	dl = dlink_create();
	dlink_add_tail(ip, dl, &opm_scan_que);
	pthread_cond_broadcast(&opm_thread->cond);
#endif /* HAVE_PTHREADS */
	return;
}

void opm_exit (void *args)
{
        return;
}

void opm_entry(void *args)
{
	
    OPM_REMOTE_T *remote;
    OPM_T *scanner;
    OPM_ERR_T err;
	int sig;
	sigset_t set;
	char *ip;
	unsigned int i, s;
	dlink_list opm_scan_set;
	
	dlink_node *dl, *tdl, *cdl;
	
	
    int scantimeout = 15;
    time_t opm_scan_timeout =  time(NULL) + 120;
			
	scanner = opm_create();

	sigemptyset(&set);
    sigaddset(&set, SIGPIPE);

    /* Setup callbacks */
    opm_callback(scanner, OPM_CALLBACK_OPENPROXY, &open_proxy, NULL);
    opm_callback(scanner, OPM_CALLBACK_NEGFAIL, &negotiation_failed, NULL);
    opm_callback(scanner, OPM_CALLBACK_TIMEOUT, &opm_timeout, NULL);
    opm_callback(scanner, OPM_CALLBACK_END, &end, NULL);
    opm_callback(scanner, OPM_CALLBACK_ERROR, &handle_error, NULL);


    /* Setup the scanner configuration */
    opm_config(scanner, OPM_CONFIG_FD_LIMIT, &fdlimit);
	opm_config(scanner, OPM_CONFIG_SCAN_IP, CfgSettings.local_ip);
	 
	//check for valid vhost in our config settings
	if (CfgSettings.local_ip) 
		opm_config(scanner, OPM_CONFIG_BIND_IP, CfgSettings.local_ip);
	else
		opm_config(scanner, OPM_CONFIG_BIND_IP, "127.0.0.1");
		
    opm_config(scanner, OPM_CONFIG_SCAN_PORT, &port);
    opm_config(scanner, OPM_CONFIG_TIMEOUT, &scantimeout);
    opm_config(scanner, OPM_CONFIG_MAX_READ, &max_read);

    /* Setup the protocol configuration */
    for (i = 0; http_ports[i]; i++)
            opm_addtype(scanner, OPM_TYPE_HTTP, http_ports[i]);


    for (i = 0; wingate_ports[i]; i++)
            opm_addtype(scanner, OPM_TYPE_WINGATE, wingate_ports[i]);


    for (i = 0; router_ports[i]; i++)
            opm_addtype(scanner, OPM_TYPE_ROUTER, router_ports[i]);


    for (i = 0; socks4_ports[i]; i++)
            opm_addtype(scanner, OPM_TYPE_SOCKS4, socks4_ports[i]);


    for (s = ARRAY_SIZEOF(socks5_ports), i = 0; i < s; i++)
            opm_addtype(scanner, OPM_TYPE_SOCKS5, socks5_ports[i]);


    for (i = 0; httppost_ports[i]; i++)
            opm_addtype(scanner, OPM_TYPE_HTTPPOST, httppost_ports[i]);
	
	opm_scan_set.head = opm_scan_set.tail = NULL;

	opm_state = 1;

#ifdef HAVE_PTHREADS
	while (sync_state != SHUTDOWN)
	{
		//try to grab out mutex
		while (opm_thread_wait) 
			sleep(5);
		mutex_state(opm_thread, THREAD_MUTEX_LOCK);
		DLINK_FOREACH_SAFE(dl,tdl, opm_scan_que.head) {
			ip = dl->data;
			dlink_delete(dl, &bl_scan_head);
			dlink_add_tail(ip, dl, &opm_scan_set);
		}		
		opm_thread_wait = 1;
		mutex_state(opm_thread, THREAD_MUTEX_UNLOCK);

		dl = tdl = NULL;
		ip = NULL;

		DLINK_FOREACH_SAFE(dl,tdl, opm_scan_set.head) 
		{
			
			ip = dl->data;
			if (!*ip)
				continue;
				
			remote = opm_remote_create(ip);
			if (!remote) 
				break;
			alog(LOG_DEBUG, "Begining proxy scan on: %s", ip);	
			switch (err_code = opm_scan(scanner, remote))
			{
				case OPM_SUCCESS:
					opm_scan_timeout = time(NULL) + 60;
					while (opm_active(scanner) > 0)
					{
						if (sigpending(&set)) 
								sigwait(&set, &sig);			
						if (time(NULL) >= opm_scan_timeout)
						{
							opm_end(scanner, remote);
							break;
						}               
						opm_cycle(scanner);
						sleep(1);
					}       
					break;
				case OPM_ERR_BADADDR:
					break;
				default:
					sendto_logchan("OPM Error: Unknown Error %d", err_code);
					break;
			}
			if (remote)
				opm_remote_free(remote);
			dlink_delete(dl, &opm_scan_set);
			dlink_free(dl);
			if (ip)
			    free(ip);
		}
	}
#endif /* HAVE_PTHREAD */
	if (scanner)
		opm_free(scanner);

#ifdef HAVE_PTHREAD
	dest_thread();
#endif
	return;
}


void open_proxy(OPM_T *scanner, OPM_REMOTE_T *remote, int notused, void *data)
{
   USE_VAR(notused);
   USE_VAR(data);
	//printf("Found proxy Port: %d\n",remote->port);
   sendto_logchan("\002Proxy:\002 Found proxy on %s:%d", remote->ip,
         remote->port);
    ircd_xline("Z", s_Guardian->nick, remote->ip, 0, "Detected possible proxy from %s:%d.", remote->ip,remote->port);
    return; //meh don't continue keep matching.
}


void negotiation_failed(OPM_T *scanner, OPM_REMOTE_T *remote, int notused,
      void *data)
{
	
    USE_VAR(scanner);
    USE_VAR(notused);
    USE_VAR(data);
	//printf("Opm negotiation failed Port: %d\n",remote->port);
    return;
}

void opm_timeout(OPM_T *scanner, OPM_REMOTE_T *remote, int notused, void *data)
{
	//printf("OPM Scan timeout Port: %d\n",remote->port);
    USE_VAR(scanner);
    USE_VAR(notused);
    USE_VAR(data);	
	return;
}



void end(OPM_T *scanner, OPM_REMOTE_T *remote, int notused, void *data)
{

   USE_VAR(scanner);
   USE_VAR(notused);
   USE_VAR(data);
//printf("OPM Scan End\n");
   return;
}

void handle_error(OPM_T *scanner, OPM_REMOTE_T *remote, int err, void *data)
{

   USE_VAR(scanner);
   USE_VAR(data);
	
   switch(err)
   {
      case OPM_ERR_MAX_READ:
			 //sendto_logchan("OPM Error: Reached maxed read on %s:%d", remote->ip,
             //  remote->port);
            break;
      case OPM_ERR_BIND:
			 //sendto_logchan("OPM Error: Unable to bind to IP/Port when attempting OPM scan.");
            break;
      case OPM_ERR_NOFD:
            //sendto_logchan("OPM Error: Unable to allocate file descriptor for %s:%d",
            //   remote->ip, remote->port);
            break;
      default:
          //sendto_logchan("OPM Error: Unknown error on %s:%d, err = %d", remote->ip,
		  //remote->port, err);
			break;
   }
}


