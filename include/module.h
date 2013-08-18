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
 *    $Id: module.h 2257M 2013-07-06 21:17:41Z (local) $
 */

#ifndef __MODULE_H__
#define __MODULE_H__

//These are custom error strings.
#define MOD_ERR_DL -2
#define MOD_ERR_MSG -1

//Module isn't in error state.
#define MOD_ERR_OK 0

//core defined Error Strings.
#define MOD_ERR_STOP 1
#define MOD_ERR_CRIT 2
#define MOD_ERR_FATAL 3
#define MOD_ERR_EXISTS 4
#define MOD_ERR_NOFILE 5
#define MOD_ERR_IO 6
#define MOD_ERR_API 7
#define MOD_ERR_ABI 8
#define MOD_ERR_MEMORY 9
#define MOD_ERR_UNLOAD 10
#define MOD_ERR_SYNTAX 11
#define MOD_ERR_UNSAFE 12
#define MOD_ERR_NOMOD  13
#define MOD_ERR_SQL    14
#define MOD_ERR_DEPENDENCY 15



//Quick reference module errors... designed for ease of use.
#define MOD_CONT MOD_ERR_OK
#define MOD_STOP MOD_ERR_STOP
#define MOD_CRIT MOD_ERR_CRIT
#define MOD_FATAL MOD_ERR_FATAL
#define MOD_MEMORY MOD_ERR_MEMORY

enum ModTypes {
  MOD_TYPE_START,
	MOD_TYPE_UNKNOWN, MOD_TYPE_PROTOCOL,
	MOD_TYPE_CORE, MOD_TYPE_3RD,
	MOD_TYPE_DB,
   MOD_TYPE_END
};

char mod_err_msg[512];


#define module_dependency(mod) \
	if (!(module_find(mod))) \
		return MOD_ERR_DEPENDENCY;

#define throwModErr(msg, ...) \
		snprintf(mod_err_msg, sizeof(mod_err_msg), msg, __VA_ARGS__); \
		return MOD_ERR_MSG; \


/**
 * This just looks ugly but i wanted to take into account
 * for directories not having a / and having a /
 * So with this said :) this is what i came up with - Twitch
 */

#define format_filename(dest,filestr)  \
	char format_file_tmp[MAXPATH]; \
	snprintf(format_file_tmp, sizeof(format_file_tmp), "%s%s", (filestr), (strstr(filestr,".so"))? "" : ".so"); \
    snprintf((dest), sizeof(dest) - 1, "%s/%s",find_module_dir((format_file_tmp)), (format_file_tmp)); \
	memset(format_file_tmp,0,sizeof(format_file_tmp));

/**
  * Module API Stuff
  *   This logic kinda looks off at first glance however we need this to make sure
  *   that module API's match, note you can manually change the ABI version on a
  *   module however do that at your own risk, as the module ABI specifies which
  *   modules the core will work properly with.
  */

#define API_MAJOR  0
#define API_MINOR  6
#define API_PATCH  7

#define API_VERSION ((MOD_API_MAJOR * 1000) + (MOD_API_MINOR * 100))
#define API_VERSION_FULL ((MOD_API_MAJOR * 1000) + (MOD_API_MINOR * 100) + MOD_API_PATH)

#define MODULE_API(maj,min,patch) ((maj * 1000) + (min * 100) + patch)

/* MAKE_ABI
 * @depreciated I realize this isn't the correct terminology, API is a better
 *              word for this, as its not the binary interface we are looking for
 *
 * XXX Remove this once its no longer in use
 */

#define MAKE_ABI(maj,min,patch) ((maj * 1000) + (min * 100) + patch)

/**
  *   Module Header
  *   	This struct allows the core to pull in the necessary information
  *   	and place it into the module structure.
  *
  *  @param name - The module name -> This is just for personal reference
  *                          as the core relies on absolute paths to find and remove
  *                         modules.
  *  @param version - The modules version in free form
  *  @param abi      - The Binary interface version, this is the version of the API
  *                            used to interact with this module.
  *  @param load     - The function called by the core when loading the module
  *  @param unload  - The function used when unloading a module from the core.
  *
  */

#define MODHEADER(name,version,author,abi,load,unload) \
ModuleInfo ModInfo = {          \
	name, version, author, abi, \
	load, unload \
} \



int nomodules;

/****
 * Necessary defines for systems without them
 */

#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif
#ifndef RTLD_LAZY
#define RTLD_LAZY RTLD_NOW
#endif
#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL 0
#endif
#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif

/***
 * Module Actions
 */

#define MOD_ACT_LOAD 0
#define MOD_ACT_UNLOAD 1
#define MOD_ACT_RELOAD 2

/***
 * Module Load Order
 */

#define MOD_LOAD_NONE -1 //no module load order
#define MOD_LOAD_PRE 0
#define MOD_LOAD_STD 1
#define MOD_LOAD_POST 2


/************************************************/

/**
 * ModInfo - This structure holds module specific information, this is information
 * 		 that is not held to be true at all times, since the modules interact with it
 *                 given that it is only used for basic checks :)
 *
 *  @param name - The module name -> This is just for personal reference
 *                          as the core relies on absolute paths to find and remove
 *                         modules.
 *  @param version - The modules version in free form
 *  @param abi      - The Binary interface version, this is the version of the API
 *                            used to interact with this module.
 *  @param mod_register   - The function called by the core when loading the module,
 *				    and takes place in the registration phase of the load.
 *
 *	NOTE: this is a required function - Modules will fail to load  if this is not specified.
 *
 *  @param mod_unregister  - The function used when unloading a module from the core,
 *				      takes place before the dlclose function is called, in the un-register phase
 *
 *	NOTE: This function is not required modules, however will not unload if this is not specified.
 *
 */

typedef struct moduleinfo_ {
     char *name;        //name of module (Modules are allowed to specify there own name)

     char *version;     //Version information in a string - This has no impact on weather or

     char *author;      //Who wrote the module

     long api;          //BINARY INTERFACE version

     int      (*mod_register)	(void);	    /* Register function;   */

	 void	  (*mod_unregister)	(void);		/* Unregister function.	*/


} ModuleInfo;


/**
 *     Module structures (Module)
 *       This holds primary module information, such as the handle, pathway, and time the module was loaded.
 *	     this information is to be held true and trusted at all times, as the MODULE system is the only thing
 *	     that is allowed to touch this information :)
 */

typedef struct module_ {

    time_t age;            	//Time we were loaded

	char name[MAXPATH]; 			 	//name of the module for the core purposes.
    char *file;

    void* handle;

    int type;               //Module types

    ModuleInfo *mi;

} Module;


/***********************************/
/**
 * Module Que Structures
 */


typedef struct moduleq_entry {
	char name[256];
	int action;
	int load;
} ModuleQEntry;

/***********************************/
/**
 * Module Linked List's
 */


dlink_list  modules;
dlink_list moduleque;


/***********************************/


int load_protocol();
int load_modules();


/*************************************/

void 	init_modules ();
void 	purge_modules();

int     module_open   (char *, int);
Module* module_find   (char *);
int     module_exists (char *);
void    module_free   (Module *);
int 	module_close  (char *);
int		module_loaddir(char *, int);


char *find_module_dir(char *module);
char *create_mod_temp(char *);

/************************************/
/**
 * Module Que Functions
 */

ModuleQEntry* find_mod_que(char *);
int addto_mod_que(char *, int, int);
int run_mod_que(int);

/************************************/
/**
 * Event Hooks
 */



#endif /*__MODULE_H__*/


