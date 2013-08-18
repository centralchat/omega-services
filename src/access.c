/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008 Omega Dev Team
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
 *    $Id: access.c 2366 2012-02-27 05:38:02Z twitch $
 */


#include "stdinc.h"
#include "server.h"


static int evh_access_rehash(int, void*);

/*******************************************/
/**
 * find_access() - Find the FIRST matching access
 * access list entry for the given user and return
 * the access list entry.
 *
 * @param u - Pointer to the user struct
 *            that we are checking with.
 *
 * @return Access* - return a pointer to the
 *                   access entry list that the user
 *                   that the user belongs to, or
 *                   NULL if no match is found.
 *
 */


Access* find_access(User *u)
{
	dlink_node *dl,*hdl;
	char *host;
	Access *a;
	char host_pattern[150];
	char ip_pattern[150];
	char nick_pattern[250];
	
	if (strlen(u->access) > 4)
			return NULL; //no need to waist extra cycles we HAVE access already.
	
       snprintf(host_pattern,sizeof(host_pattern),"%s@%s",(u->user)? u->user : "*",u->host);
       snprintf(ip_pattern,sizeof(ip_pattern),"%s@%s",(u->user)? u->user : "*", u->ip);
	
	DLINK_FOREACH(dl,accesslist.head)
	{
		a = dl->data;

		DLINK_FOREACH(hdl,a->hosts.head)
		{
		 	if (!(host = hdl->data))
					return NULL;
			if (strchr(host,'!'))
			{
					memset(nick_pattern,0,sizeof(nick_pattern));
					snprintf(nick_pattern,sizeof(nick_pattern),"%s!%s",u->nick,host_pattern);
					
					alog(LOG_DEBUG3, "find_access(%s):[HOST] checking (%s) => (%s)", u->nick, nick_pattern, host);
					
					if ((match_wild(host,nick_pattern,0)) || (match_wild(nick_pattern,host,0)))
						return a;
						
					memset(nick_pattern,0,sizeof(nick_pattern));
					snprintf(nick_pattern,sizeof(nick_pattern),"%s!%s",u->nick,ip_pattern);
				
					alog(LOG_DEBUG3, "find_access(%s):[IP] checking (%s) => (%s)", u->nick, nick_pattern, host);
					if ((match_wild(host,nick_pattern,0)) || (match_wild(nick_pattern,host,0)))
						return a;
						
			} else {
				alog(LOG_DEBUG3, "find_access(%s):[HOST] checking (%s) => (%s:%s)", u->nick, host_pattern, a->name, host);
				alog(LOG_DEBUG3, "find_access(%s):[IP] checking (%s) => (%s:%s)", u->nick, ip_pattern, a->name, host);
				if (match_wild(host,host_pattern,0)) {
					alog(LOG_DEBUG3, "find_access(%s):Found match (%s)", u->nick, host);
					return a;
				}
				if (match_wild(host,ip_pattern,0))
					return a;
			}
			continue;
		}
	}
	alog(LOG_DEBUG3, "find_access(%s): No match found (%s)", u->nick, host);
	return NULL;
}


/*******************************************/
/**
 * Check a users access against the specified level.
 *
 * @param u A pointer to the user structure which
 *          we are checking against.
 * @param level The level to check for.
 *
 * @return Return true if the access match, or return
 *         0 if we fail for any other reason.
 */


int check_access(User* u, char *level)
{

	if (!u)
		return 0;

	if (!level)
		return 1;

	//user has no access
	if (!u->access)
		return 0;

	if (strchr(u->access,*level))
		return 1;

	return 0;


}

/*******************************************/
/**
 * setAccess() - finds then sets or removes
 * access for a user, this will set the first
 * matching access level on a user, as provided '
 * by find_access().
 *
 *  @param u - The user struct we are working with
 *
 *
 *  @return int - Returns true if access has been set
 *                or false if no record was found.
 */


int setAccess(User* u)
{
    Access *a;

    if ((!u) || (u->service))
		return 0;

    memset(u->access,0,sizeof(u->access));
    
    if (!strstr(u->access,ACC_FLAG_USER))
    	strncat(u->access, ACC_FLAG_USER, sizeof(u->access));
    	  
    if (!(a = find_access (u))) 
        return 0;

    if ((!u->oper) && (CfgSettings.operonly))
		return 0;

    else { 
		if (!CfgSettings.operonly)
			strncat(u->access, ACC_FLAG_OPER, sizeof(u->access)-strlen(u->access));
		else if (u->oper) {
			 if (!strstr(u->access,ACC_FLAG_OPER))
					strncat (u->access, ACC_FLAG_OPER, sizeof(u->access));	
		}
    }
    add_metadata_user(u, "using-access", a->name);
    strncat (u->access, a->flags, sizeof(u->access)-strlen(u->access));
    return 1;

}

/*******************************************/

Access *access_exists(char *name)
{
		dlink_node *dl;
		Access *a;
		
		DLINK_FOREACH(dl,accesslist.head)
		{
				a = dl->data;
				if (strcasecmp(a->name,name)==0)
					return a;
		
		}
		return NULL;
}

/*******************************************/

Access *new_access(char *name)
{
		Access *a;
		dlink_node *dl;
		
		if ((a = access_exists(name)))
				return a;

                alog (LOG_DEBUG, "new_access(): Creating new access for %s", name);
	 			
		dl = dlink_create();
		if (!(a  = (Access *) malloc(sizeof(Access))))
				return NULL;
		
		memset(a->name, '\0',sizeof(a->name));
		memset(a->flags,'\0',sizeof(a->flags));
	        memset(a->cname,'\0',sizeof(a->cname));
		
		strlcpy(a->name,name,sizeof(a->name));
		strlcpy(a->flags,	ACC_USER,	sizeof(a->flags));
		
		a->hosts.head	= a->hosts.tail	= NULL;
	        a->non_config = 0; 
		dlink_add_tail (a, dl, &accesslist);
		return a;
}


/*******************************************/

int add_access_host(Access* a, char *host) 
{
	dlink_node *dl;
	
	char m_tmp[100];
	char *tmp, *start;
		
	if (!a)
		return 0;
	if (!host)
		return -1;
			
	memset(m_tmp, 0, sizeof(m_tmp));
	alog(LOG_DEBUG3, "add_access_host(%s): recieved %s", a->name, host);
	for (tmp = host; *tmp; tmp++) //this primarily allows us to skip ' ' or ',' quickly.
	{
		start = tmp;
		while (*tmp == ' ')
			tmp++;

		while ((*tmp != ',') && *tmp) {
			if ((*tmp == ' ') || (*(tmp+1) == '\002'))
				break;
			tmp++;
		}
		if (*start == ' ')
			start++;
		
		strncpy(m_tmp, start, tmp - start);
		
		if (!strchr(m_tmp, '@'))
			break;
		
		dlink_add_tail(strdup(m_tmp), dlink_create(), &a->hosts);
		alog(LOG_DEBUG3, "add_access_host(%s): %s", a->name, m_tmp);
		memset(m_tmp, 0, sizeof(m_tmp));
	} 
	return 1;
}

/**
 * Use pointer logic in the future
 */

int set_access_flag(Access *a, char *flags)
{
	AccessFlag *af;
	dlink_node *dl;
	char *f, *pos, *ptr;

       memset(a->flags,'\0',sizeof(a->flags));
       if ((af = find_access_flag(flags)))
		f = af->flag;	
       else
		f = flags;
	ptr = a->flags;
	while (*f) {
		if (!strchr(ptr, *f))  {
			*ptr = *f;
			ptr++;
		}	
		f++;
	}
	*ptr = '\0';
	alog (LOG_DEBUG, "set_access_flag(): handling flags for %s (Flag: %s)(AccessFlags: %s)", a->name, (af)? af->flag : flags , a->flags);
	return 1;
}

/*************************************************/
/**
 *   Destroy all entries within the access list.
 *
 *   @note This destroys all access list entries
 *         with that said, it shouldn't be called
 *         in any other circumstances besides rehash
 *         and shutdown, as it leaves us with no entries.
 *
 *
 */


int DestAccList()
{
	dlink_node *dl;
	dlink_node *tdl;

	dlink_node *hdl,*thdl; //host dlink list
	char *host;

	Access *a;

	DLINK_FOREACH_SAFE(dl,tdl,accesslist.head)
	{

		a = dl->data;
		if ((a->non_config) && (sync_state == RUNNING)) 
			continue; //skip 
		DLINK_FOREACH_SAFE(hdl,thdl,a->hosts.head)
		{
			host = hdl->data;

			dlink_find_delete(host,&a->hosts);
			dlink_free(hdl);
			
			if (host)
				free(host);
		}
		dlink_find_delete(a,&accesslist);
		dlink_free(dl);
		free(a);

	}
	return 0;

}

/*************************************************/
/**
 *   refresh_access() - Crawls our user list and checks
 *   access on each entries host.
 *
 *   @note This method can be slower on larger nets and
 *   is designed only to update access on special instances
 *   such as rehash.
 *
 *  @XXX Crawling this big of a list is ridiculous... i think we should keep a master list somewhere
 * -Twitch
 */

void refresh_access(void)
{
    User* u = NULL;
    Access* a = NULL;

    dlink_node *udl = NULL;


     DLINK_FOREACH(udl,userlist.head)
     {
            u = udl->data;

	/**
	 * if there not an OPER or they are a SERVICE
         * they have no access. so skip that entry :)
         * this should give us back some speed and
         * performance here. Esp on networks with more
         * Services and Users then IRCops (Which should
         * be all of them imo)
         * - Twitch
	 */

		if (((!u->oper) && (CfgSettings.operonly)) || (u->service))
			continue;

		setAccess(u);
     }

    return;
}
/*************************************************/

int add_access_flag (char *name,char *flag)
{
		dlink_node *dl;
		AccessFlag *af;

        	alog (LOG_DEBUG, "add_access_flag(): %s / %s", name, flag);
		
		if ((af = find_access_flag(name)))
				return -1;
		if ((af = find_access_flag(flag)))
				return -1;
				
		af = (AccessFlag*) malloc(sizeof(AccessFlag));
		if (!af)
				return 0;
		
		dl = dlink_create();
		
		strlcpy(af->name,name,sizeof(af->name));
		strlcpy(af->flag,flag,sizeof(af->flag));
		dlink_add_tail (af, dl, &access_flags);
		return 1;
}

/*************************************************/

void init_access(void)
	{
	
		add_access_flag("ROOT",	 ACC_ROOT);
		add_access_flag("SOPER",  ACC_ADMN);
		add_access_flag("SRA",	 ACC_ADMN);
		add_access_flag("ADMIN", ACC_ADMN);
		add_access_flag("STAFF",  ACC_OPER);
		add_access_flag("OPER",   ACC_OPER);
		
		build_access_list();
		AddEvent("REHASH",evh_access_rehash);
}

Access* add_access(char *name, char *host, char *flags, int non_config) {
	Access* a;
	if (!host || !name || !flags)
		return NULL;
	if (!(a = new_access(name)))
		return NULL; 
	a->non_config = non_config;
	set_access_flag(a, flags);	 
	add_access_host(a, host); 
	return a;
}
int del_access(char *name) {
	dlink_node *dl, *tdl; 
	Access *a;
        char *host;	
 	if (!*name)
		return -1;	
	if (!(a = access_exists(name)))
		return 0;
	DLINK_FOREACH_SAFE(dl, tdl, a->hosts.head)
	{
		host = dl->data;
		dlink_delete(dl, &a->hosts);
		dlink_free(dl);
		free(host);
	} 
	dl = NULL;
	dl = dlink_find_delete(a, &accesslist);
	dlink_free(dl);
	free(a);
	return 1;
}
void build_access_list() 
{
		dlink_list *config_info;
		dlink_node *dl, *tdl;
		ConfBase *cb;

		Access *a;
		char *start, *ce;
		char tmp[200];

		//this is ugly but for now its better then our constant looping.
		//their has to be a better way to do this.
		int name_hash = (int) HASH("name", HASH_B);
		int host_hash = (int)  HASH("host", HASH_B); 
		int flags_hash = (int) HASH("flags", HASH_B);

		config_info = get_config_base("access");
		DLINK_FOREACH_SAFE(dl, tdl, config_info->head)
		{
				cb = dl->data;
				if (cb->map[name_hash][0])
						a = new_access(cb->map[name_hash]);
				if (cb->map[flags_hash][0])
						set_access_flag(a,cb->map[flags_hash]);
				if (cb->map[host_hash][0]) 
			            		add_access_host(a, cb->map[host_hash]); 
		}
}

/***********************************************/
/**
 * Return an access flag, both by name or by value
 * 
 * @param name string Name or value of the flag we are looking for
 * @return AccessFlag* pointer to the flag structure.
 *
 */
AccessFlag *find_access_flag(char *name)
{
		dlink_node *dl;
		AccessFlag *af;
		
		if (!name)
			return NULL;
 
        	alog (LOG_DEBUG, "find_access_flag(): searching for %s", name);

		DLINK_FOREACH(dl,access_flags.head)
		{
				af = dl->data;
				if (strcasecmp(name,af->name)==0)
					return af;
		}
		return NULL;
}


AccessFlag *find_flag_byvalue(int flag)
{

		dlink_node *dl;
		AccessFlag *af;
		
		DLINK_FOREACH(dl,access_flags.head)
		{
				af = dl->data;
				
				if (strlen(af->flag) > 1)
					continue;
					
				if (*af->flag == flag)
					return af;
		}
	return NULL;
}

/***********************************************/

int del_acces_flag(char *name)
{

		dlink_node *dl;
		AccessFlag *af;
		
		if (!name)
			return 0;
			
		if ((af = find_access_flag(name)))
		{
			dl  = dlink_find_delete (af, &userlist);
			dlink_free (dl);
			free(af);
			return 1;
		}
		
		return 0;
}

 static int evh_access_rehash(int ac, void *av)
 {
 	DestAccList();	
	build_access_list();
	refresh_access();
	return 0;
 }
 
 /* EOF */

