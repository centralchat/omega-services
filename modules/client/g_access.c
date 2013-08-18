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
#include "extern.h"

#define NAME "g_access"
#define VERSION "$Id: g_core.c 1592 2009-12-18 06:19:21Z jer $"
#define AUTHOR "Twitch"

static void cmd_access    (User*, int, char **);
static void help_gcore     (User*);
static void help_access_ext   (User*);

static int  gbasic_load();
static void gbasic_close();


void load_access();
void save_access();

dlink_list db_access_list;



MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),gbasic_load, gbasic_close);

static int gbasic_load()
{
	  int ch;
          module_dependency("g_core");
	  ch = AddCmd(s_Guardian, "ACCESS",    ACC_FLAG_ACCESS, cmd_access, help_access_ext, 1);
      	  AddHelp(s_Guardian,help_gcore);
	  load_access();
	  return MOD_CONT;


}


static void gbasic_close()
{
	DelCmd(s_Guardian,"ACCESS",    cmd_access,  help_access_ext);
	DelHelp(s_Guardian,help_gcore);
	return;

}


/**********************************************/

static void cmd_access(User* u, int ac, char** params)
{
	dlink_node *dl,*hdl;
	Access* ca, *a;
	char* host;
	
	if (ac < 1)
		goto list;
		
	if (strcasecmp(params[0], "ADD")==0) {
			if (ac <= 3) {
                   		   sendto_one(s_Guardian, u, "Invalid syntax."); 
				   return;
			} 
			if (!(a = add_access(params[1], params[2], (*params[3])? params[3] : "oper", 1))) {
					sendto_one(s_Guardian, u, "Unable to add access for %s perhaps it already exists?",params[1]);
					return;
			}
			add_db_entry("AC", params[1], "%s %s", params[3], params[2]);
			sendto_one(s_Guardian,u, "Added access for %s (%s) [%s]", params[1], params[2], params[3]);
			refresh_access();

	} 
	else if (strcasecmp(params[0], "DEL")==0) {
		if (!(del_access(params[1]))) {
			sendto_one(s_Guardian,u,"Unable to delete access for %s (No such record found)",params[1]);
			return;
		}
		sendto_one(s_Guardian,u, "Deleted access for %s", params[1]);
		del_db_entry("AC", params[1]);
		refresh_access();

	}
	else
		goto list;
		
	return;
list:
	DLINK_FOREACH(dl,accesslist.head)
	{
	    ca = dl->data;

		sendto_one(s_Guardian,u,"%s %s [%s]",ca->name,(ca->non_config)? "<db>" : "<config>", ca->flags);
		DLINK_FOREACH(hdl,ca->hosts.head) {
				host = hdl->data;
				sendto_one(s_Guardian,u,"- %s",host);
		}
	}
	return;
}


void save_access(void)
{
	dlink_node *dl, *hdl;
	Access *ca;
	char tmp_host[1024];
	char *host;
	
	DLINK_FOREACH(dl,accesslist.head)
	{
	    ca = dl->data;
		if (!ca->non_config)
				continue;
		DLINK_FOREACH(hdl,ca->hosts.head)
		{
				host = hdl->data;
				if (tmp_host[0])
					snprintf(tmp_host, 1024, "%s", host);
				else
					snprintf(tmp_host, 1024, "%s,%s", host, tmp_host);
		}
		add_db_entry("AC", ca->name, "%s %s", ca->flags, tmp_host);
		memset(tmp_host, 0, sizeof(tmp_host));
	}
	return;
}

void load_access (void)
{
	dlink_node    *dl;
	Access 	     *a;
	DBEntry 	     *de;
	char             *params[15];
	DBContainer  *dc = NULL; 
	int	 	     n   = 0;
	char 	     *val = NULL;
	
	 if (!(dc = get_db_rows("AC"))) 
	 	return;
	
	DLINK_FOREACH(dl, dc->entries.head) 
	{
			de = dl->data;	
			val = strdup(de->value);
			
			n = generictoken (' ', 15,  val, params);
			if (n < 2)
					continue;
			if (!(a = new_access (de->key)))
					continue;
		
			add_access_host (a, params[1]);
			set_access_flag (a, params[0]);			
			
			a->non_config = 1;
			free(val);
			val = NULL;
    }
    return;
}

/************************************************/
/**
 * Help functions
 */

static void help_gcore (User* u)
{
		
		sendto_one_help(s_Guardian,u,"ACCESS",  "Manage the internal access list");	
	return;
}

static void help_access_ext (User* u)
{
	sendto_one(s_Guardian,u,"Syntax: \002ACCESS [OPTION]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"\002ACCESS ADD [name] [host] [flags]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"    Add a user to the internal access list.");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"\002ACCESS DEL [name]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"    Add a user to the access list.");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"\002ACCESS LIST\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"     List the entries in the current internal access list");
	sendto_one(s_Guardian,u," ");
	return;
}


