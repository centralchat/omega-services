/**
 *           OMEGA SECURITY SERVICES
 *            (C) 2008-2012 Omega Team
 *
 * Please read COPYING and README for further details.
 *
 *
 *  This module tracks sessions from an IP/HOST
 *  and limits them based on the set number to limit to.
 *  The idea behind this is to prevent a IP/HOST from having
 *  to many connections to the network, this can be indicative
 *  of a botnet. I am however concerned that this module
 *  has the possibility to slow omega down, since we aren't thread
 *  safe the core will need to wait for this to return, given
 *  the size of EXEMPT list will be the size of our slow down.
 *  since everything else is done with hash's and we wont take
 *  a performance hit from that... Twitch
 *
 */


#include "stdinc.h"
#include "server.h"

#define NAME "m_sessions"
#define VERSION "$Id: g_sessions.c 2394 2012-06-30 15:56:22Z twitch $"
#define AUTHOR "twitch"

int Module_Init();
void Module_Close();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7),Module_Init, Module_Close);


/******************************************************/

//for now rip our CMD_HASH function
#define HOST_HASH(x,y) hash_safe(x,y)
#define SLIST_SIZE 1024

#define HOST_LIMIT 4
#define BUFSIZE	512

#ifdef HAVE_SQLITE

    struct sqlite3	*exempt_db;

#endif

int session_host_limit = 0;


static int event_usercon (int, void *);

static void cmd_sessions  (User *, int, char **);
static void cmd_exmpt     (User *, int, char **);

int evh_session_rehash(int, void*);

void init_session_list();

typedef struct sessions_ {

    time_t age;

    int    count;

    char   host[100];

} Session;

typedef struct sesexempt_ {
    time_t age;

    time_t expires;

    char host[100];

    int  count;

} SessionExempt;


dlink_list SessionsList[SLIST_SIZE];
dlink_list exemptlist;

extern int session_count;
extern size_t session_size;

static void help_sessions     (User *);
static void help_sessions_ext (User *);
static void help_exempt_ext (User *);


Session* add_session (User *);
int del_session      (User *);

int add_exempt  (char *, int);
int del_exempt  (char *);

SessionExempt *find_exempt(char *);


static void save_exemptdb (void);
static void load_exemptdb (void);

/******************************************************/

Session* add_session(User *u)
{


    dlink_node *dl;

    int index = 0;
    int found = 0;
    char *host;


    Session* s;


    if (u->service)
	    return NULL;

    index = HOST_HASH(u->host,SLIST_SIZE);
    DLINK_FOREACH(dl,SessionsList[index].head)
    {

		s = dl->data;

		if ((!s))
			break;

		if (strcasecmp(s->host,u->host)==0)
		{
			found = 1;
			break;
		}

    }
    if (!found)
    {
        dl = dlink_create();
        s = (Session*) malloc(sizeof(Session));
        s->age = time(NULL);
        strlcpy(s->host, u->host, sizeof(s->host)); 
        s->count = 0;
	dlink_add_tail(s,dl,&SessionsList[index]);    
    }
    s->count++;
    //printf("ADDING Host index: [%d] %s %d\n", index, u->host, s->count);
    return s;


}

int del_session(User *u) {

	dlink_node *dl;
	Session *s;
	char *host;


	int index = 0;
	int found = 0;
	
    	index = HOST_HASH(u->host,SLIST_SIZE);
	//printf("Looking in bin index: %d\n", index);
	DLINK_FOREACH(dl,SessionsList[index].head) {
		s = dl->data;
		//printf("Parsing entry: [%d] %s\n", index, s->host);
		if (strcasecmp(s->host,u->host)==0) {
			found = 1;
			//printf("Found entry: [%d] %s => %s [Cnt: %d][%p]\n", index, u->host, s->host, s->count, s);
			break;
		}
	}
	if (!found)
		return found;

	if (!s)
		return 0;

	if (s->count > 0)
		 s->count--;
	else {
		dl = dlink_find_delete(s,&SessionsList[index]);
		dlink_free(dl);

		free(s);
	}
     	return 1;
}




/******************************************************/

int add_exempt(char *host, int count)
{
    dlink_node *dl;
    SessionExempt *se;


    if (find_exempt(host))
            return 2;

    se = (SessionExempt*) malloc(sizeof(SessionExempt));

    if (!se)
            return 1; //malloc failed

    strlcpy(se->host,host,sizeof(se->host));
    se->count = count;
    se->age = time(NULL);

    dl = dlink_create();

    dlink_add_tail(se,dl,&exemptlist);
    add_db_entry("EX", host, "%d", count);

    return 0;

}

/********************************************************/
/**
 * del_exempt() - Delete a host from the exempt list
 *                if * is host clear all hosts from the
 *                exempt list - used on clean up :)
 *
 *  @param host - the host we are removing.
 */


int del_exempt(char *host)
{
    dlink_node *dl,*tdl;
    SessionExempt *se;
    int found = 0;

    DLINK_FOREACH_SAFE(dl,tdl,exemptlist.head)
    {

              se = dl->data;

             if ((!match_wild(se->host,host,0)) && (!match_wild(host,se->host,0)))
                    continue;

              found++;


              dl = dlink_find_delete(se,&exemptlist);
              dlink_free(dl);
              free(se);
    }

    return (found)? 1 : 0; //if we return 0 it doesn't exist :)
}


/********************************************************/
/**
 *  find_exempt() -  find the exemption by wild matching
 *                   since we use this theory we allow
 *                   them to be added/deleted in wild fashion.
 *                   and thus need to check for them in this
 *                   fashion.
 *
 *
 *  @param host - the host we are hunting for.
 *
 *
 */


SessionExempt *find_exempt(char *host)
{
    dlink_node *dl,*tdl;
    SessionExempt *se;

    DLINK_FOREACH(dl,exemptlist.head)
    {
         se = dl->data;

          if ((match_wild(se->host,host,0)))
            return  se;
    }

    return NULL;

}

/******************************************************/


static int event_usercon(int ac, void *user)
{
   User * u = user;
   Session * s = NULL;
   SessionExempt * se = NULL;


    //mother fuckers don't be calling us.
    if (ac != 1)
        return EVENT_CONT;

    //don't waist space on services hosts
    if (u->service)
        return EVENT_CONT;

    if (!(s = add_session(u)))
	   return EVENT_CONT;

    if (s->count > session_host_limit)
    {
    	if (!(se = find_exempt(s->host)))
    		sendto_logchan("\002Sessions:\002 %s has exceeded sessions limit of (%d)",s->host,session_host_limit);
    	else
    	{
    		if (s->count > se->count)
    			sendto_logchan("\002Sessions:\002 %s has exceeded sessions limit of (%d)",s->host,se->count);
    		else
    			return EVENT_CONT;
    	}
    }
    return EVENT_CONT;
}


static int event_userexit(int ac, void *user)
{
    User * u = user;
	//printf("Calling evt user exit in g_sessions\n");
	del_session(u);
        return;

}

/**************************************************/

static void cmd_exempt(User *u, int ac, char **params)
{
        SessionExempt *se;
        dlink_node *dl,*tdl;

        int index, i;
        int threshold;

        int found = 0;
         if (!*params[0])
        {
            sendto_one(s_Guardian,u,"Invalid parameter. You must specify an argument.");
            return;
        }

       	if (strcasecmp(params[0],"LIST")==0)
        {

            DLINK_FOREACH(dl,exemptlist.head)
            {
                se = dl->data;
                sendto_one(s_Guardian,u,"%s %d",se->host,se->count);
                found++;
            }

            if (!found)
                sendto_one(s_Guardian,u,"The session exemption list is currently empty");
            else
                sendto_one(s_Guardian,u,"Currently \002%d\002 %s.",found, (found > 1)? "entries" : "entry");

            return;

        } else if (strcasecmp(params[0],"ADD")==0) {

                if (!params[1] || !params[2])
                {
                   sendto_one(s_Guardian,u,"Invalid parameter. You must specify an argument.");
                   return;
                }

                //assume they know wtf they are doing :)
                if (((threshold = atoi(params[2])) <= HOST_LIMIT) || (threshold > 100)){

                       sendto_one(s_Guardian,u,"Invalid parameter. The count must be between %d and 100.",HOST_LIMIT);
                       return;
                }

                i = add_exempt(params[1],threshold);

                switch (i)
                {
                    case 0:
                        sendto_one(s_Guardian,u,"Set %s session limit to be %d",params[1],threshold);
                        break;
                    case 1:
                        sendto_one(s_Guardian,u,"ERROR: malloc failed while adding exempt");
                        break;
                    case 2:
                        sendto_one(s_Guardian,u,"Exemption for %s already exists.",params[1]);
                        break;

                }


                return;

        } else if (strcasecmp(params[0],"DEL")==0) {

            if (!params[1])
            {
                   sendto_one(s_Guardian,u,"Invalid parameter. You must specify an argument.");
                   return;
            }

            i = del_exempt(params[1]);
	    del_db_entry("EX", params[1]);

            if (i)
                sendto_one(s_Guardian,u,"Removed exemptions matching pattern %s",params[1]);
            else
                sendto_one(s_Guardian,u,"Session exempt for %s does not appear on the list.",params[1]);


            return;
        } else if (strcasecmp(params[0],"SAVE")==0) {

            sendto_one(s_Guardian,u,"Saving exemption databases");
            save_exemptdb ();

        } else if (strcasecmp(params[0],"LOAD")==0) {

            sendto_one(s_Guardian,u,"Force loading exemption databases");

            DLINK_FOREACH_SAFE(dl,tdl,exemptlist.head)
            {
                    se = dl->data;

                    dl = dlink_find_delete(se,&exemptlist);
                    dlink_free(dl);

                    if (se)
                        free(se);
            }

            load_exemptdb();
            return;

        } else {
            sendto_one(s_Guardian,u,"Invalid parameter");
            sendto_one(s_Guardian,u,"\002/msg Guardian HELP EXEMPT\002 for more information");
            return;
        }
        return;

}


static void cmd_sessions(User *u, int ac, char **params)
{
	int i = 0;
	int threshold;
	int found = 0;
	Session *s;
	dlink_node *dl;

	 if (strcasecmp(params[0], "SHOW") == 0) {
		dlink_node	*dl;
		User		*user;

		if (ac<2)
		{
			sendto_one (s_Guardian, u, "Invalid parameter, SESSION SHOW requires a hostname");
			return;
		}

		i = 0;

		DLINK_FOREACH (dl, userlist.head)
		{
			user	= (User *) dl->data;

			if ((strcasecmp (user->host, params[1])) == 0)
			{
				sendto_one (s_Guardian, u, "[%d] %s", i, user->nick);
				i++;
			}
		}
		if (!i) 
			sendto_one(s_Guardian, u, "There are no users connected from hostname \002%s\002", u->host);
		else
			sendto_one(s_Guardian, u, "Currently listing \002%d\002 users", i); 	

		return;
	} else {
       		if (ac <= 1)
            		threshold = 3;
        	else
            		threshold = atoi(params[1]);

		if ((threshold <= 1) || (threshold > 100))
		{
			sendto_one(s_Guardian,u,"Invalid parameter, threshold must be between 2 and 100");
			return;
		}
		for (i = 0; i < SLIST_SIZE; i++) {
			DLINK_FOREACH(dl,SessionsList[i].head)
			{
				if (!(s = dl->data))
					break;

				if (s->count < threshold)
					continue;

				sendto_one(s_Guardian, u, "%s %d",s->host,s->count);
				found++;
			}		
		}
		if (!found)
				sendto_one(s_Guardian, u, "There are no hostnames connected with \002%d\002 or more sessions.", threshold);
			else
				sendto_one(s_Guardian, u, "Currently listing \002%d\002 sessions", found); 
		return;
       }
}


void init_session_list()
{
        int i;
	memset(SessionsList,0,sizeof(SessionsList));

        exemptlist.head = exemptlist.tail = NULL;

	session_count = 0;
	session_size = 0;
        return;
}

/*****************************************************/
/**
 * DB functions
 */

static void save_exemptdb(void)
{
	SessionExempt *se;
	dlink_node *dl;
	
	DLINK_FOREACH (dl, exemptlist.head)
	{
		se	= (SessionExempt*) dl->data;

		if ((!se) || (!se->host))
			continue;
		alog(LOG_DEBUG, "Saving exempts: %s %d", se->host, se->count);
	}
	return;
}


static void load_exemptdb (void)
{
	dlink_node	*dl,*tdl;
	SessionExempt   *se;
	DBEntry *de;
	
	DBContainer *dc = NULL;
	 if (!(dc = get_db_rows("EX"))) 
	 	return;
	 	
		
	DLINK_FOREACH(tdl, dc->entries.head) 
	{
		de = tdl->data;	
	    	se	= (SessionExempt*) malloc (sizeof(SessionExempt));
            	dl	= dlink_create ();
            	strlcpy(se->host,de->key,sizeof(se->host));         
            	se->count = atoi(de->value);
		se->age = time(NULL);
		dlink_add_tail (se, dl, &exemptlist);
    }
    return;

}

/*****************************************************/


static void help_sessions_ext(User *u)
{
        sendto_one(s_Guardian,u,"Syntax: \002SESSION [OPTION] [PARAMS]\002");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"\002SESSION LIST [THRESHOLD]\002");
	sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"     List all currently connected hosts with sessions at least the value");
        sendto_one(s_Guardian,u,"     of THRESHOLD");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"\002SESSION SHOW [HOST]\002");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"     Show the nicks of the currently sessions for HOST");
        sendto_one(s_Guardian,u," ");
        return;
}

static void help_exempt_ext(User *u)
{

        sendto_one(s_Guardian,u,"\002Syntax: EXEMPT [ADD|DEL|LIST] [options]\002");
        sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"\002EXEMPT LIST\002");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"     List the current sessions exemption list.");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"\002EXEMPT ADD [HOST] [COUNT]\002");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"    Allows admins to add exemptions of COUNT for a given HOST,");
        sendto_one(s_Guardian,u,"    allowing certain machines to have more sessions then the default");
        sendto_one(s_Guardian,u,"    would previously allow.");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"\002EXEMPT DEL [HOST]\002");
        sendto_one(s_Guardian,u," ");
        sendto_one(s_Guardian,u,"    Allows admins to remove exemptions from a given HOST,");
        sendto_one(s_Guardian,u,"    this will reset the connection limit for the given HOST");
        sendto_one(s_Guardian,u,"    back to the default values.");
        sendto_one(s_Guardian,u,"  ");
        sendto_one(s_Guardian,u,"   Note: This does not kill off any users already connected to");
        sendto_one(s_Guardian,u,"   the network. Also if * is specified it will remove all");
        sendto_one(s_Guardian,u,"   exempts in the list.");
        sendto_one(s_Guardian,u," ");
        return;


}
static void help_sessions(User *u)
{
        sendto_one_help(s_Guardian,u, "SESSION", "Manage and view the sessions list.");
        sendto_one_help(s_Guardian,u,   "EXEMPT", "Manipulate the sessions exempt list.");
        return;
}


int evh_session_rehash(int ac, void* args) {
   

    dlink_list *config_info;
    dlink_node *dl, *tdl;
    ConfBase *cb;
    char *ce;

     if ((ce = (char *)get_config_entry("sessions", "limit")))
		session_host_limit = atoi(ce);
     else 
		session_host_limit = HOST_LIMIT;

    config_info = get_config_base("exempt");
    DLINK_FOREACH(tdl, config_info->head)
    {
			cb = tdl->data;
			add_exempt(cb->map[HASH("host", HASH_B)], atoi(cb->map[HASH("limit", HASH_B)]));
    }
    return 0;
}
/******************************************************/
/**
 * Module factories
 * 
 * <sessions limit="">
 * <exempt host="blah" limit="100">
 */



int Module_Init()
{
    int ch;

    AddEvent("USERCONN",event_usercon);
    AddEvent("USEREXIT",event_userexit);
    AddEvent("REHASH", evh_session_rehash);

    ch = AddCmd(s_Guardian,"SESSION",ACC_ADMN,cmd_sessions, help_sessions_ext, 1);
    ch = AddCmd(s_Guardian,"EXEMPT",ACC_ADMN,cmd_exempt, help_exempt_ext, 1);
    AddHelp(s_Guardian,help_sessions);

    init_session_list();
    load_exemptdb();
    evh_session_rehash(0, NULL);

    return MOD_CONT;
}


void Module_Close()
{

    dlink_node *dl,*tdl;
    SessionExempt *se;
    Session *s;

    int i = 0;


    Log("[%s] unloading module",NAME);


    DelEvent("USERCONN",event_usercon);
    DelEvent("USEREXIT",event_userexit);
    DelEvent("USERCONN",event_usercon);
   

    del_exempt("*"); //clear our exempt list.
    for (i = 0; i <= SLIST_SIZE; i++)
    {

	DLINK_FOREACH_SAFE(dl,tdl,SessionsList[i].head)
	{
		if (!(s = dl->data))
			break;


		dl = dlink_find_delete(s,&SessionsList[i]);
		dlink_free(dl);
		free(s);
	}

	i++;
    }
    DLINK_FOREACH_SAFE(dl,tdl,exemptlist.head) {
    		if (!(s = dl->data))
    			break;
    		  dlink_delete (dl, &exemptlist);
      		  dlink_free (dl);
       		 free(se);
    }
    DelCmd  (s_Guardian,"SESSION",cmd_sessions, help_sessions_ext);
    DelCmd  (s_Guardian,"EXEMPT", cmd_exempt, help_exempt_ext);
    DelHelp (s_Guardian, help_sessions);
    return;
}


