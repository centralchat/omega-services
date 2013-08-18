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
 *    $Id: cmds.c 2376 2012-03-03 22:34:49Z twitch $
 */



#include "stdinc.h"
#include "server.h"



const float max_load_factor = 0.65;

extern Module* currentModule;


int hc = 0;

void HelpNull(User*);


/******************************************************/
/**
 * Add a command to our clients command hash.
 *
 * @note There is a bit of presumptions we make when issuing this
 * function, for one we assume that u is always our client,
 * since client structures are the same as user structures except
 * they have been flagged as our clients.
 *
 * @param u Pointer to our clients structure
 * @param name The name of the command we are using, this is important
 *        because the command will be issued and looked up via its name.
 * @param access Access level required to issue the command.
 * @param fnct A pointer to the commands function.
 *
 * @return True/False - Returns true if everything goes as planned and false if not.
 *
 */

int  AddCmd (User *u, char *name,char *access, void (*fnct)(User*,int, char**), void (*hlp)(User*), int log)
{

	CmdHash *ch;	
        dlink_node *dl;	
	if (!u)
	 	return 0;

	struct cmdbase_ *cb = (struct cmdbase_ *) malloc(sizeof(struct cmdbase_));	

	if (!cb)
		return 0;

	int index = HASH(u->nick,CMD_HASH_KEY);
	
	if (!UserCmdHash[index]) {
		UserCmdHash[index] = (CmdHash *) malloc(sizeof(CmdHash));

		strlcpy(UserCmdHash[index]->name, u->nick, sizeof(UserCmdHash[index]->name));
        UserCmdHash[index]->command_help_shrt.head = NULL;
        UserCmdHash[index]->command_help_shrt.tail = NULL; 	

		UserCmdHash[index]->commands.head = NULL; 
		UserCmdHash[index]->commands.tail = NULL;

        UserCmdHash[index]->entry_cnt = 0;	
	}

	strlcpy(cb->name, name, sizeof(cb->name));
	cb->hashlp = 0;
	cb->hascmd = 0;
	
	if (access)
		strlcpy(cb->access, access, sizeof(cb->access));
	
	if (fnct) {	
		cb->fnct = fnct;
		cb->hascmd = 1;
	}

	if (hlp) {	
		cb->hashlpext = 1;
		cb->ext_help = hlp;
	}
   	alog(DEBUG3,"Adding CLIENT [%s] ... command %s (%s) (Location: %p)", u->nick, cb->name, cb->access, fnct);
	dl = dlink_create();	
	dlink_add_tail(cb,dl,&UserCmdHash[index]->commands);
    UserCmdHash[index]->entry_cnt++;	
    return index;
}


/*****************************************************/

int AddHelp(User* u, void (*helpcmd)(User*))
{



   HelpEntry *ch,*ch2;
   dlink_node *dl;
   int index = 0;
   
   if (!u)
		return -1;
   alog(DEBUG3,"Adding help (Location: %p)",helpcmd);
   index = HASH(u->nick, CMD_HASH_KEY);
   dl = dlink_create();
   ch = (HelpEntry*) malloc(sizeof(HelpEntry));
   ch->routine = helpcmd;
   //dlink_add_tail (tmp, dl, &userlist);
   dlink_add_tail(ch,dl,&cmd_help[index]);
   return 1;
}

int DelHelp(User *u, void (*helpcmd)(User*))
{

	HelpEntry* he;
	dlink_node *dl;

	if (!u)
	 	return 0;

        int index = HASH(u->nick, CMD_HASH_KEY);

	DLINK_FOREACH(dl, cmd_help[index].head)
	{
		he = dl->data;
		if (he->routine != helpcmd)
			continue;
		dlink_delete(dl,&cmd_help[index]);
		dlink_free(dl);
		if (he)
		    free(he);
		break;
	}
	return 0;
}


/******************************************************/
/**
 *  void DelCmd() - Remove a command from our 
 *                  clients user structure.
 *
 * @param u         = The user struct that this command belongs to.
 * @param fnct      = Pointer to the commands handler.
 * @param hlp       = Pointer to the extended help function.
 *
 */

void DelCmd(User* u, char* cmd, void (*fnct)(User*, int, char**), void (*hlp)(User*))
{
    dlink_node *dl,*tdl;
    CmdHash *ch; 
    struct cmdbase_ *cb;
    if (!u)
	 	return;
    int index = HASH(u->nick,1024);
    ch = UserCmdHash[index];
	if (!ch)
		return;
    DLINK_FOREACH_SAFE(dl,tdl,ch->commands.head)
    {
        cb = dl->data;
        if (cb->fnct != fnct)
            continue;
        dlink_delete(dl,&ch->commands);
        dlink_free(dl);
        ch->entry_cnt--;
        free(cb);
        break;
    }  
    if (!ch->entry_cnt)
       UserCmdHash[index] = NULL;
    return;
}


/*****************************************************/
/**
 * int do_cmd() - Handles a command to a client -
 *                do all the sanity checks here.
 *
 *  @param cmd         = The command we are actioning on.
 *  @param us          = A pointer to OUR client struct
 *  @param u           = A pointer to the SRC of the command
 *  @param ac          = The argument count being passed into the command
 *                       handler - used to see if av is null :)
 *  @param av          = The PARAMETERS to the command - the original
 *                       command.
 *  @return int
 *             0 = Success
 *             1 = No cmd for it was found.
 *             2 = Access Denied.
 *             3 = Fail something was passed wrong
 * @TODO 
 *      - Write this so that it can make more sense in the future
 * 		- Also find a better way to handle user commands, i fear that this isn't 
 *        the best method of doing this... meh.
 */



int do_cmd(char *cmd, User* us, User *u, int ac, char **av)
{
 	   CmdHash* ch;
	   struct cmdbase_ *c;
	   HelpEntry *he;
       dlink_node *dl;
	   int index = 0, found = 0;
	   int t = 0;
	   
 	   if (!us || !u)
 	     	return 0;

	   index = HASH(us->nick,CMD_HASH_KEY);
	   if (strcasecmp(cmd,"HELP")==0)
	   {
		
		if (ac > 0)   {
		
			if (!(ch = UserCmdHash[index])) {
				sendto_one(us, u, "There is currently no help available for that command.");
				return CMD_ERR_OK;
			}
			DLINK_FOREACH(dl,ch->commands.head)  {
         			c = dl->data;
				if (strcasecmp(c->name,av[0])==0) {
					found = 1;
					c->ext_help(u);
					sendto_one(us, u, "Limited to access flag: \002%s\002", c->access);
				}
			}
			if (!found)
				sendto_one(us, u, "There is currently no help available for that command.");	
			return CMD_ERR_OK;	
		} 
		DLINK_FOREACH(dl, cmd_help[index].head) {
			he = dl->data;
			if (he->routine)
				he->routine(u);
 		}
		return CMD_ERR_OK;				 
	   }
    
	   if (!(ch = UserCmdHash[index]))
		return CMD_ERR_INDEX;

	DLINK_FOREACH(dl,ch->commands.head)  {
         	c = dl->data;
          if (strcasecmp(c->name,cmd)==0) {
                   if (!check_access(u, c->access)) {
                       sendto_one(us,u,"Permission Denied.");
		       		return 1;
		   }
           if (!c->fnct)
               continue; //no command pointer found bail
		 	c->fnct(u, ac, av);
		 	if ((logcmds) || (debug > 1) || (c->log))
				sendto_logchan("[%s] %s: %s",us->nick,u->nick,cmd);
         	return 0;
         }
	}
    return 1;
}



/*****************************************************/

int HELP_FOREACH(User* us, User *u)
{
  	return 0;
}



/*****************************************************/


void ACCESS_DENIED(User* s,User *u) { return; }


char *CmdErr(int err)
{
	const char *cmd_err_codes[] = {
		"Command success, no error.",
		"Invalid command - No such command found.",
		"Permission Denied.",
		"Command Error, Invalid paramaters.",
		"Command Error, Invalid index.",
		"Command Error, HASH does not exsist."
	};

	return (char*) cmd_err_codes[err];
}

/*****************************************************/


void HelpNull(User *u) { return; }

/*****************************************************/

void HandleClientCmds(User* u, int ac, char **av)
{
   	User* us;
	char *params[15];
	char *cmd;
	int rc = 0, n = 0;
	char *ptr = NULL;
	char *pos = NULL;
	
	if ((!av[2]) || (!u))
		return;
    if (*av[1] == '#')
		return;
	if (*av[2] == '\001' || *av[2] == '\01' || *av[2] == '\1')
		return;
    if (!(us = find_user(av[1])))
        return;

	for (n = 0; n <= 15; n++)
        params[n] = NULL;

	n = 0;
	cmd = strdup(av[2]);
	if ((ptr = strchr(cmd, ' ')) != NULL) {
		*ptr++ = '\0'; //break after command
		while (1) {
			params[n++] = ptr;	
		 	if ((pos = strchr(ptr, ' ')) != NULL) {					
				*pos++ = '\0';
				ptr = pos;
			} else 		
				break;
		}
	}
	rc = do_cmd(cmd, us, u, n, params);
	if ((rc) && (rc < 3)) {
		sendto_one(us,u,"%s",CmdErr(rc));
		return;
	}
	return;
}
