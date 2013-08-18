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
 *    $Id: eventhandler.c 2396 2012-06-30 16:53:15Z twitch $
 */

#include "stdinc.h"
#include "server.h"

extern Events* event_list;
extern Module* currentModule;

dlink_list servcmds;
dlink_list usercmds;

void destroy_event_list()
{
	dlink_node *dl, *tdl;
	Events *ev;
	int i = 0;
	
	alog(LOG_EVENT,"Cleaning up event list for exit.");
	DLINK_FOREACH_SAFE(dl, tdl, events.head) {
        ev  = dl->data;
		
		dlink_delete (dl, &events);
		dlink_free (dl);
		free (ev);
		i++;
	}
	alog(LOG_EVENT,"Destroyed (%d) events.",i);
	return;
}

/*****************************************/

int AddEventEx(char *event, int every, int howmany, int (*fnct)(int,  void *))
{
    Events      *tmp;
    dlink_node  *dl;

    tmp = (Events*)malloc(sizeof(Events));

    if (!tmp)
    {
		alog(3,"Unable to add event %s malloc failed",event);
        return 0;
    }

    memcounts.eventssize    += sizeof(tmp);
    memcounts.eventscnt++;

    strlcpy (tmp->name, event, sizeof(tmp->name));

    tmp->handler = fnct;
    tmp->age = time(NULL);

    if (every > 0)
    {
        tmp->interval   = every;
        tmp->when       = time(NULL) + every;
    } else
        tmp->interval   = -1;

	tmp->recall = howmany; //since 0 = remove -1 = unlimited.

    dl  = dlink_create ();

    dlink_add_tail (tmp, dl, &events);

    return 0;
}

void DelEvent(char *event, void* fnct)
{

       Events      *tmp;
       dlink_node  *dl;


    DLINK_FOREACH(dl, events.head) {
	   tmp = dl->data; 
    	if (!fnct) {
    		if ((strcasecmp (tmp->name, event)) != 0)
    			continue;
    	} 
        else {
    		if (tmp->handler != fnct)
    			continue;
    	}

    	memcounts.eventssize    -= sizeof(tmp);
    	memcounts.eventscnt--;

    	dlink_delete (dl, &events);
    	dlink_free (dl);
    	free (tmp);
    	return;
    }
    return;
}

/*****************************************/

void RunTimedEvents ()
{
    dlink_node  *dl, *tdl;
    Events      *ev;

    if (!ServerTime)
        return;

    DLINK_FOREACH_SAFE(dl, tdl, events.head)
    {
        ev  = dl->data;

        if (ev->interval != -1)
        {
			if (ev->recall == 0)
			{
				dlink_delete (dl, &events);
				dlink_free (dl);
				free (ev);
				continue;
			}

            if (ev->when <= ServerTime)
            {
				ev->last 	= ev->when;
                ev->when    = ServerTime + ev->interval;
                ev->handler (0, NULL); // Timed events do not need parameters

                if (ev->recall > 0)
                    ev->recall--;
            }
        }
    }

    return;
}

/*****************************************/

void EventEx(char *event, int argc, void *argv, int (*error_handler)(char *, Events*, int, void*))
{
    Events      *ev;
    dlink_node  *dl, *tdl;
    int status  = EVENT_OK;
    int event_count = 0;

    if (!event)
        return;

    if (debug)
		alog (5, "Emitting Event %s", event);

    DLINK_FOREACH_SAFE (dl, tdl, events.head)
    {
        ev  = dl->data;
        event_count++;
	    if (ev->interval > 0) 		
       		continue; 
	    if ((strcasecmp (ev->name, event)) == 0)
		{
			alog(DEBUG3,"Found Match for event: %s Recall: %d", event, ev->recall);
			
			if ((ev->recall != 0) || (ev->recall == -1)) 
            { 
				status = ev->handler (argc, argv);
                switch (status) 
                {
                    case EVENT_OK:
                    case EVENT_CONT:
                        break;
                    case EVENT_STOP:
                    case EVENT_ABORT:
                        alog(LOG_EVENT, "Event returned STOP halting event chain for %s", event);
                        status = -1;
                        if (error_handler)
                        {
                           if (error_handler(event, ev, argc, argv))
                               status = EVENT_OK;
                        } 
                        break;       
                }
            }
			if (status < 0)
                break;

			if (ev->recall > 0)
				ev->recall--;	
			
			if (ev->recall == 0)
			{
				dlink_delete(dl,&events);
				dlink_free(dl);
				free(ev);
				continue;
			} 
		}
	}
    alog(LOG_EVENT, "Event run %s ran %d events in chain", (status >= 0)? "successfull" : "failure", event_count);
	return;
}

/*****************************************/

void AddServCmd(char *cmd,void (*fnct)(Link*,int,char**))
{

    dlink_node *dl  = dlink_create ();
    ServCmds *c = (ServCmds*) malloc(sizeof(ServCmds));
    int index = HASH(cmd,1024);

    strcpy(c->name, cmd);
    c->fnct = fnct;

    dlink_add_tail(c, dl, &ServCmdTable[index]);

}

/*****************************************/

int HandleServCmd(char *command, Link* li, int arc, char **arv)
{
    ServCmds *c;
    dlink_node *dl;
    int found = 0;
    int index = HASH(command,1024);

    DLINK_FOREACH(dl,ServCmdTable[index].head)
    {
            c = dl->data;

            if (!c || !c->name || !command)
                continue;

            if (strcasecmp(c->name, command)==0)
            {
		    found = 1;
            	    c->fnct(li,arc,arv);
	     }
    }

    if (!found)
		alog(DEBUG3,"Handling unknown server command %s",command);


    return 0;

}

/**********************************************/

void DelServCmd(char *cmd,void (*fnct)(Link*,int,char**))
{
        ServCmds* c;
        dlink_node* dl;

        int index = HASH(cmd,1024);

        DLINK_FOREACH(dl,ServCmdTable[index].head)
        {
            c = dl->data;

            if (c->fnct == fnct)
            {

                dl = dlink_find_delete(c,&ServCmdTable[index]);
                dlink_free(dl);

                //free(c->module);
                free(c);

                return;

            }

        }

    return;
}



/**********************************************/
/**
 * Okay instead of using a fat dlinked list
 * index the commands by name into a hash
 * then retrieve associated lists
 * this will save time in the future.
 */


void AddUserCmd(char *cmd,void (*fnct)(User*,int,char**))
{

    UserCmds* uc;
    dlink_node *dl;
    int index = HASH(cmd,1024);

    dl = dlink_create();

    if (!(uc = (UserCmds*) malloc(sizeof(UserCmds))))
        return;

    uc->routine = fnct;
    uc->active = 1;


    dlink_add_tail(uc,dl,&UserCmdTable[index]);

    return;

}

void DelUserCmd(char *cmd,void (*fnct)(User*,int,char**))
{
    UserCmds* uc;
    dlink_node* dl;

    int index = HASH(cmd,1024);

    DLINK_FOREACH(dl,UserCmdTable[index].head)
    {
            uc = dl->data;

            if (uc->routine == fnct)
            {
                    dl = dlink_find_delete(uc,&UserCmdTable[index]);
                    dlink_free(dl);
                    uc->active = 0;

                    /*
                    if (uc->module) {
                        fprintf (stderr, "freeing uc->module %p\n", uc->module);
                        free(uc->module);
                    }
                    */
                    if (uc)
                        free(uc);
                    return;

            }
    }

}



/*****************************************/

int HandleUserCmd(char *command, User *u, int arc, char **arv)
{

    int rc = 1;
	int index;

    UserCmds* uc;
    dlink_node* dl;

    if (!u || arc <= 0 || !arv)
        return 0;

    index = HASH(command,1024);

    DLINK_FOREACH(dl,UserCmdTable[index].head)
    {
            uc = dl->data;

	     assert(uc);

	     if (!uc)
		  continue;

	     if (!uc->routine)
			return 0;

         uc->routine(u,arc,arv);
    }


	return 0;
}

int ev_housekeeping (int argc, void *nil)
{
    return EVENT_OK;
}

int ev_connectuplink (int argc, void *nil)
{
    if (!servsock)
    {
        printf ("No socket connected\n");
        return EVENT_CONT;
    }

    if (servsock->sd != -1)
    {
        printf ("Socket already connected as fd %d\n", servsock->sd);
        return EVENT_CONT;
    }

    if (!Connect(servsock, CfgSettings.uplink, CfgSettings.local_ip, CfgSettings.port))
    {
        alog(LOG_ERROR, "Failed to connect uplink\n");
        return EVENT_CONT;
    }

	DelTimedEvent ("Connect Uplink");
    ircd_connect ();
    
	return EVENT_OK;
}



