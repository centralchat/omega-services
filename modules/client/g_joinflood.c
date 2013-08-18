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
 *   Holds Client command hash functions.  This is for client commands so
 *   clients do not need to wade through to find there own commands.
 *
 *    $Id: g_joinflood.c 2394 2012-06-30 15:56:22Z twitch $
 */

#include "stdinc.h"
#include "server.h"
#include "extern.h"


#define NAME "g_joinflood"
#define AUTHOR "Twitch"
#define VERSION "$Id: g_joinflood.c 2394 2012-06-30 15:56:22Z twitch $"

dlink_list floodchans;

typedef struct floodchan_ {

    char name[34];

    time_t join_duration;

    time_t join_floodend;

    int joincnt;

    int jflood;

} FloodChan;

int join_duration = 5;
int join_flood_cnt = 10;


void m_sjoin (Link*, int , char **);
FloodChan* Find_FloodChan(char* chan);
int chk_floods(int, void*);

static int joinflood_load();
static void joinflood_close();


MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),joinflood_load, joinflood_close);


int joinflood_load()
{
	  char *ce;
	  int delim = 0;
	  char tmp[10];
	  char *start;
	  
	  
      AddServCmd ("SJOIN", m_sjoin);
      AddTimedEvent ("JOIN FLOOD", chk_floods, 5);

      floodchans.head = floodchans.tail = NULL;
	  
	  if ((ce = (char *)get_config_entry("flood", "join")))
	  {
				start = ce;
				memset(tmp, 0, sizeof(tmp));
		  
				while (*ce)
				{
						switch (*ce)
						{
								case ':':
									if (delim == 0)
									{
										strncpy(tmp,start, ce - start);
										join_flood_cnt = atoi(tmp);
										break;
									} 
									if (delim == 1) 
									{
										strncpy(tmp,start, ce - start);
										join_duration = time_to_sec(tmp);
										break;
									}
									break;
						}
						ce++;
				}
			
	  }
      return MOD_ERR_OK;
}

void joinflood_close()
{
	DelServCmd("SJOIN",m_sjoin);
	DelTimedEvent("JOIN FLOOD");
	return;
}

void m_sjoin (Link *li, int argc, char **argv)
{
   	int found = 0;
	time_t	thisjoin = 0;
	FloodChan* c = NULL;
	dlink_node* dl;

    if (sync_state != 3)
            return;

    thisjoin = time(NULL);

    //add a channel setup the info (we will delete this out later)
    if (!(c = Find_FloodChan(argv[2])))
    {
            
        dl = dlink_create();

        if (!(c = (FloodChan*) malloc(sizeof(FloodChan))))
        	return;


        strlcpy(c->name,argv[2],sizeof(c->name));

        c->join_duration = time(NULL) + join_duration;
        c->joincnt = 1;
        c->jflood = 0;

        dlink_add_tail (c, dl, &floodchans);

        return;
    }
    
   if (c->jflood == 1)
           return;

    if (thisjoin < c->join_duration)
        c->joincnt++;

    if (thisjoin >= c->join_duration) {
            c->join_duration = time(NULL) + join_duration; //this should be TIME + duration
            c->joincnt = 1;
            return;
    } else {

        if (c->joincnt < join_flood_cnt)
                return;

		c->jflood = 1;

		ircd_wallop(s_Guardian->nick,"\002Flood Detected:\002 Detected JOIN/PART flood in %s (%d joins in %lu seconds)",
					c->name,c->joincnt, join_duration);

		sendto_logchan("\002Flood:\002 %s has been join/part flooded (%d joins in %lu seconds)",
					c->name,c->joincnt, join_duration);
					
		send_line(":%s JOIN %s",(IRCd.ts6)? s_Guardian->uid : s_Guardian->nick, c->name);
        send_line(":%s MODE %s :+iRsm",s_Guardian->nick,c->name);

        c->join_floodend = time(NULL) + 120; //hold the flood for 2minutes

        return;
	}
	return;
}


int chk_floods(int ac, void *nil)
{
	dlink_node *dl,*tdl;
	FloodChan* c;
	time_t thistime = time(NULL);

  	DLINK_FOREACH_SAFE (dl, tdl, floodchans.head)
    {
       	c   = dl->data;
        if ((c->jflood == 1) && (thistime > c->join_floodend))
        {
        	send_line(":%s MODE %s -iRsm",(IRCd.ts6)? s_Guardian->uid : s_Guardian->nick,c->name);
			sendto_logchan("\002Flood:\002 Removing FLOOD status from %s",c->name);
			send_line(":%s PART %s :Network Security Services",(IRCd.ts6)? s_Guardian->uid : s_Guardian->nick,c->name);

 			//reset channel statistics
			dl  = dlink_find_delete (c,&floodchans);
            dlink_free (dl);
            free(c);
		}
        //clean up the list.
		else if (c->join_floodend < thistime)
		{

            dl  = dlink_find_delete (c,&floodchans);
            dlink_free (dl);
            free(c);
            return EVENT_OK;
		}
   	}
    return EVENT_OK;
}


FloodChan* Find_FloodChan(char* chan)
{
    dlink_node* dl;
    FloodChan*   c;

    DLINK_FOREACH (dl, floodchans.head)
    {
        c = dl->data;

        if ((strcasecmp (c->name, chan)) == 0)
            return c;
    }

    return NULL;
}

