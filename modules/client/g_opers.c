
#include "stdinc.h"
#include "server.h"
#include "extern.h"

#define NAME    "g_opers"
#define VERSION "$Id: g_opers.c 7 2008-07-09 11:51:00Z Stranded $"
#define AUTHOR  "Stranded and Jer"


static void cmd_opers    (User*, int, char**);
static void cmd_protect  (User*, int, char**);

static void help_opers_ext  (User*);
static void help_opers      (User*);
static void help_protect    (User*);
static void help_protect_ext(User*);

static int Module_Init();
static void Module_Close();


static char *protect_modes;

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7), Module_Init, Module_Close);


static int Module_Init()
{
        int ch; char *ce;
        AddCmd(s_Guardian, "OPERS",    ACC_FLAG_USER, cmd_opers, help_opers_ext, 1);
        AddCmd(s_Guardian, "PROTECT",  ACC_FLAG_OPER, cmd_protect, help_protect_ext, 1);
        
        if ((ce = (char *)get_config_entry("protect", "modes")))
		protect_modes = ce;
	else
		protect_modes = "Os";
		
	AddHelp(s_Guardian,help_opers);	   
	AddHelp(s_Guardian,help_protect);
	return MOD_CONT;
}

static void Module_Close()
{
        DelCmd(s_Guardian, "OPERS", cmd_opers,help_opers_ext);
        DelCmd(s_Guardian, "PROTECT", cmd_protect, help_protect_ext);
	
	DelHelp(s_Guardian,help_opers);
	DelHelp(s_Guardian,help_protect);
	
        return;
}



static void help_opers(User* u)
{
        sendto_one_help(s_Guardian,u, "OPERS", "IRC Operators list");
        return;
}

static void help_opers_ext(User* u)
{
        sendto_one(s_Guardian,u,"Syntax: \002OPERS\002");
        sendto_one(s_Guardian,u,"-");
        sendto_one(s_Guardian,u,"List all currently online IRC Operators connected");
        sendto_one(s_Guardian,u,"to the network.");
		return; //always return
}

/****************************************************/

static void cmd_opers  (User* src, int ac, char** av)
{
 	User *u;	
        dlink_node *dl;
        sendto_one(s_Guardian, src, "IRC Operators connected to the network");
        DLINK_FOREACH (dl, userlist.head)
	{
      	  	 u = dl->data;
        	if ((u->oper) && (!u->service)) {
        		sendto_one (s_Guardian, src, "\002%s\002 connected from %s [%s]", 
						u->nick, u->serv->name, (u->admin)? "Server Adminastrator" : "IRC Operator");       	
		
		}
        }
        return;
}


static void cmd_protect (User* src, int ac, char** argv)
{
       int n;
       Channel* c;
       char *chan;
       
       if (!*argv[0]) {
       	   	sendto_one(s_Guardian,src,"Invalid Paramaters - This command requires a paramater.");
       		return;
       }
       for (n = 0; n < ac; n++)
       {
       		chan  = argv[n];
       		if (*chan == '-')
       			chan++;
		if (!(c = find_channel(chan)))  {
				sendto_one(s_Guardian,src,"Channel %s does not exsist.", chan);
				return;
		}  
		switch (*argv[n]) {
			case '-':
				ircd_mode(s_Guardian, c, "-%s", protect_modes);
				sendto_one(s_Guardian,src,"Removing modes %s on channel %s",c->name, protect_modes);
				break;
			default:
		 	      ircd_mode(s_Guardian, c, "+%s", protect_modes);
				sendto_one(s_Guardian,src,"Setting modes %s on channel %s", c->name,  protect_modes);
				break;
		}
	}
}


static void help_protect(User *u)
{
	sendto_one_help(s_Guardian,u,"PROTECT", "Sets protection modes on a channel");
	return;
}
static void help_protect_ext(User *u)
{
	sendto_one(s_Guardian,u,"Syntax: \002PROTECT [-][CHANNEL] [-][CHANNEL] ...\002");
	sendto_one(s_Guardian,u,"-");
	sendto_one(s_Guardian,u," Sets modes %s on the specified channel", protect_modes);
	sendto_one(s_Guardian,u," If - is specified prior to a channel modes will be removed.");
	sendto_one(s_Guardian,u,"-");
	return;
}
