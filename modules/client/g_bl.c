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
 * Desc: 
 *
 *    $Id$
 *
 */

#include "stdinc.h"
#include "server.h"


#define NAME "g_bl"
#define AUTHOR "Twitch"
#define VERSION "Blacklist zone checker"


static int Module_Init	();
static void Module_Close();
static int evh_blcon   (int, void*);
static void bl_thread_entry(void *args);
struct addrinfo * bl_gethost(char const *, int);


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7),Module_Init, Module_Close);


struct zone_entry {
       char host[MAXSERV];
       char reason[512];       
};

dlink_list bl_zones;
dlink_list bl_scan_que;
dlink_list hosts_tmp;


Thread *bl_thread = NULL;
int bl_thread_state = 1;
int bl_scan_count = 0;
int bl_thread_wait = 1;
int bl_lock = 0;


/*****************************************/
/**
 *  Module_Init() - Module factory called
 *                when the module is loaded.
 *   return int Any status changes the module should pass to
 *				the core... see MOD API documentation.
 */

static int Module_Init()
{
    ConfBase *cb = NULL;
    dlink_list *config_info = NULL;
    dlink_node *tdl = NULL, *dl = NULL;
    char *host = NULL;
	int i = 0;
    int index = HASH("host", HASH_B);
    int oindex = HASH("reason", HASH_B);
    struct zone_entry *zone;
    
     
#ifndef HAVE_PTHREAD
    throwModErr("Module Error, Threading is disabled or not available.","");
#endif
    
    bl_zones.head    = bl_zones.tail    = NULL;
    bl_scan_que.head = bl_scan_que.tail = NULL;
    
    AddEvent ("USERCONN", evh_blcon);
    
    if (!(config_info = get_config_base("blacklist")))
                     return MOD_STOP;
                     
   	DLINK_FOREACH(tdl, config_info->head)
	{   
           cb = tdl->data;
	       if (!cb->map[index][0])
                   continue;
           
           if (!(zone = (struct zone_entry*)malloc(sizeof(struct zone_entry))))
                            return MOD_ERR_MEMORY;
                            
           strncpy(zone->host, cb->map[index], sizeof(zone->host));
           if (cb->map[oindex][0])
                  strncpy(zone->reason, cb->map[oindex], sizeof(zone->reason));
           else
               sprintf(zone->reason, "Your ip has been found on a blacklist (%s)",cb->map[index]);
                       
           dl = dlink_create();
           dlink_add_tail(zone, dl, &bl_zones);
    	}

	// for (i = 0; i < 6; i++) {
	// 	if (!(spawn_thread(NULL, bl_thread_entry, NULL)))
	// 			break;
	// }
 //    if (!i)
	// 	return MOD_STOP;
	// bl_lock = 0;
	return MOD_CONT;

}

/**
 * Closing proceedure this is were you remove your clients
 * and commands and is called on unload.
 */
 
static void Module_Close()
{
	bl_thread_state = 0;
	return;
}


/*******************************************/


static int evh_blcon (int ac, void *user) {
    User *u = user;
    dlink_node *dl = NULL, *tdl = NULL; 
    char *host = NULL, *thost = NULL;
    unsigned char addr[16];

    if (sync_state != RUNNING)
        return EVENT_CONT;
        
    if ((u->ip[0] == '0') || (strchr(u->ip, ':')))
        return EVENT_CONT;
        
   	if (strcasecmp(u->host, "data.searchirc.org")==0)
		return EVENT_CONT;
        
    if ((match_cidr(addr, "\x7F", 8)) || (match_cidr(addr, "\0", 8)) || match_cidr(addr,"10.0.0.0", 8) ||
        match_cidr(addr,"172.16.0.0", 12)) 
		return;
	
    if (strcasecmp(u->ip, CfgSettings.local_ip)==0)
		return EVENT_CONT;
                      
	if (!(host = strdup(u->ip)))
		return EVENT_STOP;

	if (!(spawn_thread(u, bl_thread_entry, NULL))) 
	{
		alog(LOG_ERROR, "Unable to create thread for dns lookup on %s", u->nick);
		return EVENT_CONT;
	}

	// if (bl_lock) { 
	// 	dl = dlink_create();
	// 	dlink_add_tail(host, dl, &hosts_tmp);
	// 	return EVENT_CONT;	   
	// } else {
	// 	bl_thread_wait = 1;	
	// 	DLINK_FOREACH_SAFE(dl, tdl, hosts_tmp.head) {
	// 			thost = dl->data;
	// 			dlink_delete(dl, &hosts_tmp);
	// 			dlink_add_tail(thost, dl, &bl_scan_que);
	// 	}
	// }
	// dl = dlink_create();
	// dlink_add_tail(host, dl, &bl_scan_que);
	// bl_thread_wait = 0;
   return EVENT_OK;
}


/*******************************************/


static void bl_thread_entry(void *args) {
#ifdef HAVE_PTHREAD
	User *u = args;
	dlink_list bl_scan_local;
	// dlink_node *dl = NULL, *tdl = NULL;
	dlink_node *zdl = NULL;
	char host[MAXSERV+1];
	char ip[MAXSERV+1];
	int skip = 0;
	char *thost = NULL, *tmp_host = NULL;
    
	struct zone_entry *zone;
    struct sockaddr_in *sin; 
    struct addrinfo *hostlist, *ptr;
    
	memset(host, '\0', MAXSERV+1);
	
	if ((!u) || (!args))
		return;

	// while (sync_state != SHUTDOWN)
	// {
		// while ((bl_thread_wait) || (bl_lock))
		// 	sleep(1);

		// bl_lock = 1;				
		// DLINK_FOREACH_SAFE(dl,tdl, bl_scan_que.head) {
		// 	thost = dl->data;
		// 	dlink_delete(dl, &bl_scan_que);
		// 	dlink_add_tail(thost, dl, &bl_scan_local);
		// }
		// bl_lock = 0;
		// dl = tdl = NULL;
		thost = strdup(u->host);

		// DLINK_FOREACH_SAFE(dl, tdl, bl_scan_local.head) { 
			// thost = dl->data;
			reverse_ip(thost, ip);
			alog(LOG_DEBUG2, "BL: Looking for a BL entry for %s", thost);
			skip = 0;
			DLINK_FOREACH(zdl, bl_zones.head) {	
				zone = zdl->data;					
				sprintf(host, "%s.%s", ip, zone->host);
				alog(LOG_DEBUG2, "BL: checking list [%s] for entry %s", zone->host, host);
				hostlist = bl_gethost(host, 0);

				if (!hostlist) {
					alog(LOG_DEBUG2, "BL: No result found continuing");
					continue;
				}

				for (ptr = hostlist; ptr != NULL ;ptr=ptr->ai_next)
                { 
                    sin  = (struct sockaddr_in*) ptr->ai_addr;
                    sendto_logchan("\002BL:\002 Results found for %s [Reason: %s][Zone: %s]", thost, inet_ntoa(sin->sin_addr), zone->host);
                    ircd_xline("Z", s_Guardian->nick, thost, 0, "%s", zone->reason);
					skip = 1;
					break;
                }
				if (skip)
					break;
				memset(host, '\0', sizeof(host));
			}
			memset(ip, '\0', sizeof(ip));
			// dlink_delete(dl, &bl_scan_local);
			// dlink_free(dl);
			free(thost);
		// }
		// sleep(1);
	// }
	// printf("We reached end of thread on our own, noway\n");
	dest_thread(); 
#endif 
	return;
}


#ifdef HAVE_GETADDRINFO
struct addrinfo * bl_gethost(char const *host, int port)
{
  struct addrinfo hints, *res;
  int error;
  char portbuf[6];
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = 0;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  snprintf(portbuf, 6, "%d", port);
  if ((error = getaddrinfo(host, portbuf, &hints, &res)))
             return NULL;
  return res;
}

#endif




