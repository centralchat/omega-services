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
 * Desc: Module API demonstration 
 *
 *    $Id$
 *
 */

#include "stdinc.h"
#include "server.h"

/************************************************/
/**
 * Defines
 */

#define NAME "m_foo"
#define AUTHOR "Twitch"
#define VERSION "Module API Demonstration"

/************************************************/
/**
 * ProtoTypes
 */

User *s_FooServ;

static int foo_init		();
static void foo_connect ();

static void cmd_foo(User*, int, char **);

static void help_m_foo (User*);
static void help_foo_ext(User*);

static int Module_Init	();
static void Module_Close();
/**
 * The module header is required on all Modules it specifies the following
 * @param name 		(string)   - The module name
 * @param version 	(string)   - Current Module version
 * @param author 	(string)   - Author of the module
 * @param abi 		(MAKE_ABI) - The version of the binary interface that the module
 *								was designed for. This must be a MAKE_ABI returned value.
 * @param init_funct *(int)()  - Pointer to the Module factory.
 * @param close_funct *(void)() - Point to the closeing procedure.
 */
 

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,7),Module_Init, Module_Close);

/*****************************************/
/**
 *  Module_Init() - Module factory called
 *                when the module is loaded.
 *   return int Any status changes the module should pass to
 *				the core... see MOD API documentation.
 */

static int Module_Init()
{

      int status;
	  int ch;

      if (!(foo_init()))
		return MOD_STOP;
				
      //Add the command and the extended help to the command hash, also the 0 signifies
      //that this command does not require OPERONLY privs, if operonly is set to on
      AddCmd(s_FooServ, "FOO", ACC_USER, cmd_foo, help_foo_ext, 0); //add the command fo to the stack  
      AddHelp(s_FooServ,help_m_foo); //add standard help text.
      
      foo_connect();
      return MOD_CONT;
}

/**
 * Closing proceedure this is were you remove your clients
 * and commands and is called on unload.
 */
 
static void Module_Close()
{
	if (s_FooServ) {
		DelHelp(s_FooServ,help_m_foo); //delete the help from the bot
		DelClient(s_FooServ->nick); //delete our cilent
	}
	return;
}



/*************************************************/
/**
 * This function is a standard C style function
 * it returns true if our new client is created 
 */
 
static int foo_init()
{
	char nick[40];
	char host[40];
	char ident[40];
	char *ce = NULL;

	//initialize defaults of our identity
	strcpy (nick,	"FooServ");
	strcpy (host,	"im.fooserv.me");
	strcpy (ident,	"foo");
	
	//read the values from the configuration file, setting the 
	//values we need into the correct places.
	if ((ce = (char *)get_config_entry("fooserv", "nick")))	
		strlcpy (nick,	ce, sizeof(nick));
	if ((ce = (char *)get_config_entry("fooserv", "host")))
		strlcpy (host,	ce, sizeof(host));
	if ((ce = (char *)get_config_entry("fooserv", "ident")))
		strlcpy(ident,ce, sizeof(ident));

	if ((s_FooServ = NewClient(nick, ident, host, "Module API Demonstration"))) //create a new client structure for our client
		return 1; //return true

        return 0; //we couldnt create the structure so bail.
}

/*************************************************/
/**
 * Read the channels from the configuration, and join
 * client to the specified channels.
 * 
 *  side effects, client will be in channels, and 
 *  channels will be created if empty.
 */
 
static void foo_connect()
{
        Channel* c;
	char *ce = NULL, *start = NULL;
	char chan_tmp[100];

 	//join the log channel
	ircd_add_user (s_FooServ); //introduce our client
 	if (!(c = find_channel (logchan))) //check to see if log channel exists
   	{
		 c               = new_chan (logchan); //create log channel
          	 c->channelts    = time(NULL); //set internal channel ts to current time stamp.
    	}
	AddToChannelU (s_FooServ, c); //add our client to the channel. since we know it exists
	
	//begin reading the configuration and parsing out a channel list.	
	if ((ce = (char *)get_config_entry("fooserv", "channels")))
	{
			for (; *ce; ce++)
			{
				
				while (*ce == ' ')
					ce++;
				start = ce;
		
				while ((*ce != ',') && (*ce)) {
						if (*ce == ' ')
								break;
						ce++;
				}
				if (!*ce)
						break;
								
				strncpy(chan_tmp, start, ce - start);
				if (!(c = find_channel (chan_tmp)))
				{
						c		= new_chan (chan_tmp);
						c->channelts	= time(NULL);
				}

				if (!in_channel (s_FooServ, c))
						AddToChannelU (s_Guardian, c);
						
				memset(chan_tmp,0,56);
			}
	}	
	return;

}


/*****************************************/
/**
 * This is a command hook function
 * it is called when the command it is hooking 
 * is triggured.
 * @param u  (User *) - Pointer to the user structure of the person calling the command
 * @param ac (int)    - Number of parameters passed to the command, the av value is 
 *                      ac - 1 .
 * @param av (Array)  - Array of pointers broken up by parameters passed to the command
 *                      paramaters start after COMMAND 0 1 2 3 
 *
 * @return void
 */
 
static void cmd_foo(User *u, int ac, char **av)
{
	sendto_one(s_FooServ,u,"Your a fooo....!");
	return;
}

/*****************************************/
/**
 * Non extended help function, this is called when a user
 * types HELP with no parameters. Also refered to as the 
 * command list. This  is not a required entry for a command.
 * 
 * @param u (User *) - Pointer to the calling user's struct.
 */

static void help_m_foo(User *u)
{
	//Send the help back, this is basic help and is padded 
	//to fit within the list properly.
	sendto_one_help(s_FooServ,u,"FOO", "Foo Serv Help");
	return;
}

/*****************************************/
/**
 * Extended help function is ran when a user types
 * HELP COMMAND this is set with the command struct
 * and is also prefixed with the access level this command
 * is limited to. NOTE you must have access of required level
 * to view the extended command.
 *
 * @param u (User *) - Pointer to the calling user's struct.
 */
static void help_foo_ext(User *u)
{
	sendto_one(s_FooServ,u,"FOO");
	sendto_one(s_FooServ,u,"Module API Command Demonstration.");
	return;
}

