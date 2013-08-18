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
 *    $Id: module.c 2394 2012-06-30 15:56:22Z twitch $
 */

#include "stdinc.h"
#include "server.h"

int mod_event_rehash(int, void*);
int mod_event_shutdown(int, void*);

/*****************************************/
/**
 * Return a module error string.
 *
 * @param status Error code we are returning.
 *
 * @note negative value of status is representative of module
 * side errors, these are custom errors that modules
 * must define.
 */


char *GetModErr(int status)
{


if (status <= 0)
	{
		switch (status)
		{
			case MOD_ERR_MSG:
				return (char *) mod_err_msg;
		}
	}


	if (status > 15)
		return "Invalid Module Return";

	const char *ModErr[] = {
			"Success, no error.",
			"Module Error, Module Returned STOP.",
			"Module Error, CRITICAL.",
			"Module Error, FATAL.",
			"Module Error, File already loaded.",
			"Module Error, No such file.",
			"Module Error, IO Error.",
			"Module Error, Invalid API.",
			"Module Error, Module does not support current module API.",
			"Module Error, Allocating Memory.",
			"Module Error, Unable to unload module.",
			"Module Error, Invalid SYNTAX.",
			"Module Warning, Unsafe to unload.",
			"Module Error, Core set in no-module mode.",
			"Module Error, SQLite is not available on this system.",
			"Module Error, Dependency not met.",
			NULL
	};

	if (ModErr[status])
		return (char*)ModErr[status];
	else
		return (char*)"Module Warning, No return from MODULE";

}


/*****************************************/


void init_modules() 
{
    /*
     * Load modules, clients, and db's
     */

	if (debug > 0)
		printf("\nRequired Module API: %d.%d.x [\033[1;32m%d\033[0m]\n\n", API_MAJOR, API_MINOR, API_VERSION);

    if (!load_protocol())
        exit(0);
		
	if (!load_modules())
		return;
		
	run_mod_que(MOD_LOAD_PRE);
	run_mod_que(MOD_LOAD_STD);
    run_mod_que(MOD_LOAD_POST);

	AddEvent("SHUTDOWN", mod_event_shutdown);
	AddEvent("REHASH", mod_event_rehash);
	
	return;
}

/*****************************************/
/**
 * load_protocol() - Load our given protocol
 *                   if it fails toss a critical
 *                   error and return
 *  @return bool
 *              true  - Module has loaded successfully
 *              false - Module failed to load see logs
 *
 */


int load_protocol()
{
    int status = 0;
	char *ce;
	
	if (!(ce = (char *)get_config_entry("protocol", "name")))
			return 0;
			
	strlcpy(CfgSettings.protocol,ce,sizeof(CfgSettings.protocol));
	
        status = module_open(CfgSettings.protocol, MOD_TYPE_PROTOCOL);

	if ((status!= 0 && status != 12))
	{
       		sendto_console("FATAL: %s \033[1;34m %s \033[0m",CfgSettings.protocol, GetModErr(status));
		alog(LOG_FATAL,"Protocol Error: [%s] %s",CfgSettings.protocol, GetModErr(status));
		return 0;
	}

    return 1;

}
/*******************************************************/
int module_loaddir(char *dir, int order) {
	DIR *dp;
        struct dirent *ep;
	char path[PATH_MAX];
	Module* m;
	
	snprintf(path, PATH_MAX, "%s/%s", MPATH, dir);
	alog(LOG_MODULE, "Loading directory path: %s (Priority: %d)", path, order);
	if (!(dp = opendir(path)))
		return 0;

	while ((ep = readdir (dp))) {
			m = NULL;

			if (*ep->d_name == '.')
				continue;

            if (ep->d_name[strlen(ep->d_name) - 3] != '.')
                continue;


		if ((m = module_find(ep->d_name)))
			continue;
		alog(LOG_DEBUG, "module_loaddir(): adding %s to modque (Priority: %d)", ep->d_name, order);
		addto_mod_que(ep->d_name,	MOD_ACT_LOAD,	order);
	}
	(void) closedir (dp);
	return 1;
}


int load_modules()
{

	DIR *dp;
    struct dirent *ep;
	char path[255];
	Module* m;
	int index = 0;
	int dindex = 0;
	int oindex = 0;
	int order = 0;

	char *tmp, *start;
        char m_tmp[512];	
	dlink_list *config_info;
	dlink_node *dl, *tdl;
	ConfBase *cb;
	ConfEntry *ce;
	
	
	int status = 0;

	if (!(dp = opendir(CPATH)))
		return -1;
		
	if (!skip_banner)      
		fprintf(stderr,"\nLoading core modules ");

	while ((ep = readdir (dp))) {
		m = NULL;
		
		status = 0;
		
		if (*ep->d_name == '.')
			continue;

		if ((m = module_find(ep->d_name)))
			continue;
			
		
			switch ((status = module_open(ep->d_name, MOD_TYPE_CORE)))
			{
					case MOD_ERR_OK:	
							if ((sync_state != RUNNING) && (!skip_banner)) {
								fprintf(stderr,"\033[1;34m|\033[0m ");
							} 
							break;
					default:
							fprintf(stderr,"[\033[1;34mFailed\033[0m]\n");
							alog(LOG_FATAL, "Unable to load core module %s (%s)", ep->d_name, GetModErr(status));
							Exit(1);
							break;
			}
			alog(LOG_MODULE, "Loading module %s [%s]", ep->d_name, GetModErr(status));	
	}
	
	if ((sync_state != RUNNING) && (!skip_banner)) {
		fprintf(stderr," [\033[1;32mDone\033[0m]\n\n");
		fprintf(stderr,"Loading modules: \n");
	}

        module_loaddir("", MOD_LOAD_PRE);
	if (nomodules) {
		fprintf(stderr,"[\033[1;34mNote\033[0m] Services were started in skeleton mode. Non-core modules will not be loaded.\n");			
		return 1;			
	}

	index = HASH("name", HASH_B);
	dindex = HASH("dir", HASH_B);
	oindex = HASH("load", HASH_B);
	
	config_info = get_config_base("module");
	DLINK_FOREACH(tdl, config_info->head)
	{
			
			cb = tdl->data;
		 	order = MOD_LOAD_STD;	
			switch (cb->map[oindex][0])
			{
					case 'p':
						if (strcasecmp(cb->map[oindex], "POST")==0) 
							order = MOD_LOAD_POST;
						else
							order = MOD_LOAD_PRE;
						break;
					
			}
			
			if (cb->map[dindex][0]) {
					module_loaddir(cb->map[dindex], order);
					continue;
			}
			if (cb->map[index][0]) {
		  		tmp = cb->map[index];
                                if (!strchr(tmp, ' '))
                                        addto_mod_que(tmp,MOD_ACT_LOAD, order);
               			memset(m_tmp, '\0', sizeof(m_tmp));
				alog(LOG_DEBUG3, "Parsing module string: [%s]", tmp);
                                while (*tmp)
                             	{
				
					while (*tmp == ' ')
						tmp++;
					
					start = tmp;		
					while ((*tmp != ',') && (*tmp)) {
						if (*tmp == ' ')
							break;
						tmp++;
					}
					strncpy(m_tmp, start, tmp - start);
                                        addto_mod_que(m_tmp, MOD_ACT_LOAD, order);   
					alog(LOG_DEBUG3, "Parsing out module: %s", m_tmp);           
                                	memset(m_tmp, '\0', sizeof(m_tmp));  
					tmp++;  	
				}	
			}
                   }
	return 1;
}


/*******************************************************/
/**
 * Create a temporary module file.
 *  @param filename (string) Path to module file we are copying
 *  @return 		(string) Pointer to the file name that 
 *							 we just created.
 *  @note - this file name must be free'd upon delete.
 */

 
char *create_mod_temp(char *file)
{
	char output[MAXPATH];
	char input[MAXPATH];;
	char buffer[MAXREADBUFFER];
	DIR *dp;  
	FILE *src, *trg, *tmp;
	int fd, len, unused;


	memset(output, 0, MAXPATH);
	memset(input, 0, MAXPATH);
	memset(buffer, 0,  MAXREADBUFFER);
	//snprintf should return the length of written string.	  
	len = snprintf(output,MAXPATH,"%s/%s", TMP_DIR, (CfgSettings.network)? CfgSettings.network : "misconfigured");
	  
	if (!(dp = opendir(output)))
		mkdir(output, S_IRWXO | S_IRWXG | S_IRWXU);
	
	len = strlcat(output,"/TMP_", MAXPATH - len);
	len = strlcat(output,file,MAXPATH - len);
	len = strlcat(output,"XXXXXX",MAXPATH - len);
	
	format_filename(input,file); 
	
	if ((tmp = fopen(output,"r"))) {
			unlink(output);
			fclose(tmp);
	}
	if ((fd = mkstemp(output)) == -1) //since mktemp is depreciated for portability use mkstemp
	{
		printf("Make tmp failed - %s\n", strerror (errno));
		return NULL;
	}
		
	if (!(trg = fdopen(fd, "w"))) {
	      alog(LOG_ERROR, "Unable to open tempfile [%s] for writing", output);
     	  return NULL;
    }

	if (!(src = fopen(input,"r"))) {
	    alog(LOG_ERROR, "Unable to open module file [%s] for reading", file);
    	return NULL;
    }
	//write and read as much as possible at a given time.
	while (!feof(src)) {
		unused = fread(buffer, 1, MAXREADBUFFER - 1, src);
        unused = fwrite(buffer, 1, sizeof(buffer), trg);
	}
	fclose(src);
	fclose(trg);

	return strdup(output);
}


/*****************************************/

/**
  * module_open() - Load our module file symbols
  *
  * @param filename The file we are trying to link
  *              against.
  * @param type  Type of module we are loading.
  *              - 0 MOD_TYPE_UNKNOWN Unknown module type it was either loaded
  *                after or manually.
  *              - 1 MOD_TYPE_PROTOCOL
  *              - 2 MOD_TYPE_CORE
  *              - 3 MOD_TYPE_STD
  *              - 4 MOD_TYPE_CLIENT
  *
  * @return int
  *  - 0 = MOD_ERR_OK   Loading is successful no error.
  *	 - 1 = MOD_ERR_STOP We receive a stop either from the module or core.
  *  - 2 = MOD_ERR_CRIT Critical error happened while loading.
  *  - 3 = MOD_ERR_FATAL Fatal error occurred while loading.
  *  - 4 = MOD_ERR_EXISTS Module is already loaded.
  *  - 5 = MOD_ERR_NOFILE No such file/directory
  *  - 6 = MOD_ERR_IO We were unable to read symbols from the module file.
  *  - 7 = MOD_ERR_API No open/closing functions loaded
  *  - 8 = MOD_ERR_ABI In Omega ABI refers to the BINARY INTERFACE version
  *		               which is used to in dictate whether a module will run or not
  *                    if this is returned the ABI values do not match.
  *  - 9 = MOD_ERR_MEMORY We were unable to allocate memory.
  *  - 10 = MOD_ERR_UNLOAD Module file is unsafe to unload
  *  - 11 = MOD_ERR_SYNTAX Something funky was passed to us.
  *  - 13 = MOD_ERR_NOMOD Core is running in skeleton no modules cannot be loaded
  *
  *
  *  @note we don't wanna exit if a module fails just return fail status, so
  *  the caller can handle errors.
  */


int module_open(char *filename, int type)
{
  	Module* m;
	dlink_node *dl;
	char  tmp[256];
	int (*abi_version)();
	long cur_api;
	
    /*
     * If we have no structure loaded from the module we wont get a new value
     * for status. So with this in mind set this to invalid API so we know
     * this when we return and error off.
     */

	int mod_status = MOD_ERR_API;

	
   /*
    * Initialize the structures
    */
	if ((m = module_find(filename)))
		return MOD_ERR_EXISTS;
		
		
    if (!(m = (Module*) malloc(sizeof(Module))))
        return MOD_ERR_MEMORY;

    m->age = time(NULL);
    snprintf(m->name,sizeof(m->name),"%s%s",filename,(strstr(filename,".so"))? "" : ".so");
    if (!(m->file = create_mod_temp(filename)))
		return MOD_ERR_IO;

    /*
    * Open the module for loading since all
    * our sanity checks are done.
    */


    if (!(m->handle = dlopen(m->file, RTLD_NOW)))
    {
            module_free(m);
            throwModErr("%s",dlerror());
    }
    if (!(m->mi = (ModuleInfo*) dlsym(m->handle,"ModInfo")))
    {
            module_free(m);
			dlclose(m->handle);
            throwModErr("%s",dlerror());
    }
	//check for version
	if ((m->mi->api < API_VERSION) || (m->mi->api > (API_VERSION+100))) {
		cur_api = m->mi->api;
		module_free(m);
		dlclose(m->handle);
		throwModErr("Module Error, module does not support current API [Module: %d][Required: %d]", (int)cur_api, API_VERSION); 
	}
	
 	/*
    * Now all the symbols that we need are loaded,
	* are within version tolerances, and
	* placed into the module structure; Run
    * the register function.
    */
    mod_status = 0;
    if (m->mi->mod_register) {
		if ((mod_status = m->mi->mod_register()) != MOD_ERR_OK) {
			if (m->mi->mod_unregister) 
				m->mi->mod_unregister();
			module_free(m);
			dlclose(m->handle);
			return mod_status;		
		}
    } else {
    		if (m->mi->mod_unregister)
			m->mi->mod_unregister();
		dlclose(m->handle);
		module_free(m);
		return MOD_ERR_API;
    }
    m->type = type;
    dl = dlink_create();
    dlink_add_tail (m, dl, &modules);
    return MOD_ERR_OK;
}


/************************************************/


int module_exists (char * fileName)
{
    FILE *fp;   
    if ((fp = fopen(fileName,"r"))) {
		fclose(fp);
		return 1;
    }	
   return 0;

}

/**********************************************************/


char *find_module_dir(char *module)
{
	char path[255];

	snprintf(path,sizeof(path),"%s/%s",PPATH,module);

	if (module_exists(path))
		return PPATH;

	memset(path,0,sizeof(path));

	snprintf(path,sizeof(path),"%s/%s",CPATH,module);

	if (module_exists(path))
		return CPATH;

	memset(path,0,sizeof(path));

	snprintf(path,sizeof(path),"%s/%s", CLIENT_MOD_DIR, module);

	if (module_exists(path))
		return CLIENT_MOD_DIR;

	memset(path,0,sizeof(path));

	snprintf(path,sizeof(path),"%s/%s",CONTRIB_PATH,module);

	if (module_exists(path))
		return CONTRIB_PATH;

	memset(path,0,sizeof(path));

	snprintf(path,sizeof(path),"%s/%s",DBPATH,module);

	if (module_exists(path))
		return DBPATH;

	memset(path,0,sizeof(path));
	
	return MPATH; //return MPATH no matter what as our module_open will just shoot out module doesn't exists

}

/*********************************************************/


void module_free(Module *m)
{

    dlink_node *dl;

	assert (m != NULL);

    if ((dl = dlink_find_delete(m,&modules)))
        dlink_free(dl);

	if (m->file) {
		if (module_exists (m->file))
				unlink(m->file);
				
		free(m->file);
	}
	
	free(m);

}


/*****************************************/

int module_close(char *filename)
{
    Module* module;
    dlink_node *dl;

    if (!(module = module_find(filename)))
            return MOD_ERR_NOFILE;

    if (module->mi->mod_unregister) {
        module->mi->mod_unregister();
    } else {
		return MOD_ERR_UNLOAD;
    }

    alog(LOG_MODULE,"Unloading module: %s",filename);

    dlclose(module->handle);
    module_free(module);

    return MOD_ERR_OK;
}
/*****************************************/

Module* module_find(char *filename)
{
    dlink_node* dl;
    Module* m;
	char tmp[256];
	
	if (!filename)
		return NULL;	
	
    snprintf(tmp,sizeof(tmp),"%s%s",filename,(strstr(filename,".so"))? "" : ".so");
	
    DLINK_FOREACH (dl, modules.head)
    {
        m = dl->data;
        
		if (strcasecmp(m->name,tmp)==0)
						return m;
    }

    return NULL;
}

/*****************************************/

void purge_modules()
{

    dlink_node *dl,*tdl;
    Module *module;

	DLINK_FOREACH_SAFE (dl,tdl, modules.head)
	{
			module = dl->data;				
            if (module->type == MOD_TYPE_PROTOCOL)
					continue;
					
			if (module->mi->mod_unregister) 
					module->mi->mod_unregister();
			
			dlclose(module->handle);
			module_free(module);
	}
	
	/* make sure we purge protocol module last - this 
	 * looks like were looping twice but there should
	 * only be one entry in the list at this point
	 */
	DLINK_FOREACH_SAFE (dl,tdl, modules.head)
	{
			module = dl->data;								
			if (module->mi->mod_unregister) 
					module->mi->mod_unregister();
			
			dlclose(module->handle);
			module_free(module);
	}
	

}
/********************************************/
/**
 **      Module Que Operations
 **
 **/
 
ModuleQEntry *find_mod_que(char *file)
{
	dlink_node *dl;
	ModuleQEntry *mq;
	
	DLINK_FOREACH(dl,moduleque.head)
	{
			mq = dl->data;
			if (strcasecmp(mq->name,file)==0)
					return mq;
	}
	return NULL;
 }

 
int addto_mod_que(char *file,int op, int load)
{

	dlink_node *dl;
	ModuleQEntry *mq;
	
	//if ((mq = find_mod_que(file)))
	//return MOD_ERR_EXISTS;
	
	if (!(mq = (ModuleQEntry*) malloc(sizeof(ModuleQEntry))))
		return MOD_ERR_MEMORY;
	
	
	strlcpy(mq->name,file,sizeof(mq->name));
	
	mq->action = op;
	mq->load = load;
	
	dl = dlink_create();
	dlink_add_tail(mq,dl,&moduleque);
	return MOD_ERR_OK;
}

int run_mod_que(int load)
{
	dlink_node *dl,*tdl;
	ModuleQEntry *mq;
	int status;
	
	DLINK_FOREACH_SAFE(dl,tdl,moduleque.head)
	{
		mq = dl->data;
		
		if (!mq->name)
			goto freedl;
			
		switch (mq->action)
		{
			case MOD_ACT_LOAD:
				if ((mq->load != load) || (load == MOD_LOAD_NONE))
						goto cont;
						
				if ((module_find(mq->name)))
						break;
					
				status = module_open(mq->name, MOD_TYPE_UNKNOWN);

				alog(LOG_MODULE,"Loading module %s [%s]",mq->name,GetModErr(status));
				if ((sync_state != RUNNING) && (!skip_banner)) {							
					printf(" * Loading: \033[1;32m %s%s \033[0m \033[1;34m %s \033[0m \n", 
							mq->name, (strstr(mq->name, ".so"))? "" : ".so", (status != 0)? GetModErr(status) : "");
				}	
				//we have an unmet dependency, try to load it next time around. if we are on MOD_LOAD_POST then fail it completely.
				if ((mq->load != MOD_LOAD_POST) && (status == MOD_ERR_DEPENDENCY)) {
						mq->load += 1;
						goto cont;
				}
				
				break;
			case MOD_ACT_UNLOAD:
				status = module_close(mq->name);
				alog(LOG_MODULE,"Unloading module %s [%s]",mq->name,GetModErr(status));
				break;
			case MOD_ACT_RELOAD:
				if ((status = module_close(mq->name)) == 0)
					status = module_open(mq->name, MOD_TYPE_UNKNOWN);
							
				alog(LOG_MODULE,"Reloading module %s [%s]",mq->name,GetModErr(status));
				break;
		}
freedl:
		dlink_delete(dl,&moduleque);
		dlink_free(dl);
			
		if (mq)
			free(mq);
cont:
		continue;

	}
	return 1;
}

/********************************************/
/**
 **      Module Loader Event Hooks
 **
 **/
 
 
int mod_event_shutdown(int ac, void* args)
{
	purge_modules();
	return 0;
}

int mod_event_rehash(int ac, void* args)
{
	run_mod_que(MOD_LOAD_PRE);
	run_mod_que(MOD_LOAD_STD);
	run_mod_que(MOD_LOAD_POST);
	return 0;
}
