#include "setup.h"
#include "stdinclude.h"
#include "appinclude.h"

int mod_event_rehash(int, void*);
int mod_event_shutdown(int, void*);


char * module_paths[] = {
  MOD_SOCKET_PATH  ,
  MOD_PROTOCOL_PATH,
  MOD_CORE_PATH    ,
  MOD_CONTRIB_PATH ,
  MOD_DB_PATH      ,
  MOD_CLIENT_PATH  ,
  MOD_STD_PATH     ,
  NULL
};

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


char * GetModErr(int status)
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

char * mod_type_string(int mod_type) 
{
  int type = mod_type <= 0 ? 0 : mod_type;
  const char * mod_type_strings[] = {
    "Unknown",
    "Core",
    "SocketEngine",
    "Protocol",
    "3rd Party",
    "Database"
  };

  if (mod_type_strings[type])
    return (char*) mod_type_strings[type];
  else
    return (char*) mod_type_strings[0];
}


const char * mod_queue_prio_string(int prio)
{
  if (prio < 0)
    return "None";

  const char * mod_queue_prios[] = {
    "All",
    "Pre",
    "Post",
    "Std"
  };
  return mod_queue_prios[prio];
}


/*****************************************/


void modules_init() 
{
    /*
     * Load modules, clients, and db's
     */

  if (core.debug > 0)
    log_message(LOG_DEBUG, "\nRequired Module API: %d.%d.x [\033[1;32m%d\033[0m]\n\n", 
      API_MAJOR, API_MINOR, API_VERSION);


  module_loaddir_ext(MOD_CORE_PATH, MOD_QUEUE_PRIO_PRE, MOD_TYPE_CORE);


  // if (!load_modules())
  //  return;
    
  run_mod_que(MOD_QUEUE_PRIO_PRE);
  run_mod_que(MOD_QUEUE_PRIO_STD);
  run_mod_que(MOD_QUEUE_PRIO_POST);

  return;
}

/*******************************************************/
/* module_loaddir - Load a directory of modules
 * mark them as type, if type is given
 */

int __module_loaddir(char *dir, int order, int type) {
  DIR *dp;
    
  struct dirent *ep;
  char path[PATH_MAX];
  Module* m;
  
  snprintf(path, PATH_MAX, "%s/", dir);
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
    addto_mod_que_ext(ep->d_name, type, MOD_ACT_LOAD, order);

  }
  (void) closedir (dp);
  return 1;
}

/*******************************************************/


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

  

  module_loaddir("", MOD_QUEUE_PRIO_PRE);
  if (core.skeleton) {
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
    order = MOD_QUEUE_PRIO_STD; 
    switch (cb->map[oindex][0])
    {
      case 'p':
        if (strcasecmp(cb->map[oindex], "POST")==0) 
          order = MOD_QUEUE_PRIO_POST;
        else
          order = MOD_QUEUE_PRIO_PRE;
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
 *  @return     (string) Pointer to the file name that 
 *               we just created.
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
  len = snprintf(output,MAXPATH,"%s/", TMP_DIR);
    
  if (!(dp = opendir(output)))
    mkdir(output, S_IRWXO | S_IRWXG | S_IRWXU);
  
  strncat(output,"/TMP_", MAXPATH - len);
  strncat(output,file,MAXPATH - len);
  strncat(output,"XXXXXX",MAXPATH - len);
  
  format_filename(input,file); 
  
  if ((tmp = fopen(output,"r"))) {
      unlink(output);
      fclose(tmp);
  }
  if ((fd = mkstemp(output)) == -1) //since mktemp is depreciated for portability use mkstemp
  {
    log_message(LOG_ERROR, "Make tmp failed - %s\n", strerror (errno));
    return NULL;
  }
    
  if (!(trg = fdopen(fd, "w"))) {
        log_message(LOG_ERROR, "Unable to open tempfile [%s] for writing", output);
        return NULL;
    }

    log_message(LOG_DEBUG, "Attempting to open: %s", input);
  if (!(src = fopen(input,"r"))) {
      log_message(LOG_ERROR, "Unable to open module file [%s] for reading", file);
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
  *              - 1 MOD_TYPE_SOCKETENGINE
  *              - 2 MOD_TYPE_CORE
  *              - 3 MOD_TYPE_STD
  *              - 4 MOD_TYPE_CLIENT
  *              - 5 MOD_TYPE_DB
  *
  * @return int
  *  - 0 = MOD_ERR_OK   Loading is successful no error.
  *  - 1 = MOD_ERR_STOP We receive a stop either from the module or core.
  *  - 2 = MOD_ERR_CRIT Critical error happened while loading.
  *  - 3 = MOD_ERR_FATAL Fatal error occurred while loading.
  *  - 4 = MOD_ERR_EXISTS Module is already loaded.
  *  - 5 = MOD_ERR_NOFILE No such file/directory
  *  - 6 = MOD_ERR_IO We were unable to read symbols from the module file.
  *  - 7 = MOD_ERR_API No open/closing functions loaded
  *  - 8 = MOD_ERR_ABI In Omega ABI refers to the BINARY INTERFACE version
  *                  which is used to in dictate whether a module will run or not
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
    
    
    if (!(m = (Module*) calloc(1,sizeof(Module))))
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

    log_message(LOG_MODULE, "Module loaded [%s] type [%s]", 
      m->name, mod_type_string(m->type));

    return MOD_ERR_OK;
}


/************************************************/

int module_exists (char * file)
{
  FILE *fp;   
  if ((fp = fopen(file,"r"))) {
    fclose(fp);
    return 1;
  } 
  return 0;
}

/**********************************************************/


char *find_module_dir(char *module)
{
  char path[255];

  int pos = 0;

  for (; module_paths[pos]; pos++)
  {

    snprintf(path,sizeof(path),"%s/%s",module_paths[pos],module);

    if (module_exists(path))
      return module_paths[pos];

    memset(path, '\0', sizeof(path));
  }

  return MODULE_DIR; //return MODULE_DIR no matter what as our module_open will just shoot out module doesn't exists

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
/**
 * Remove a module, running its unregister
 * handler, and remove it from module list
 * TODO: Make this thread safe, check for locks
 */

int _module_close(Module * module)
{  
  log_message(LOG_MODULE,"Trying to unload module: %s",module->name);
  if (module->mi->mod_unregister)
    module->mi->mod_unregister();
  else                     //This is just incase someone forgets to define the unload function
   return MOD_ERR_UNLOAD;  // Is it truely unsafe to unload? That is the million dollar question


  dlclose(module->handle);
  
  log_message(LOG_MODULE,"Module unloaded: %s",module->name);

  module_free(module);

  return MOD_ERR_OK;
}

/*****************************************/

int module_close(char *filename)
{
    Module* module;
    if (!(module = module_find(filename)))
        return MOD_ERR_NOFILE;
   return _module_close(module);
}

/*****************************************/

Module * module_find(char *filename)
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

void modules_purge()
{

  dlink_node *dl,*tdl;
  Module *module;

  DLINK_FOREACH_SAFE (dl,tdl, modules.head)
  {
    module = dl->data;  

    if (module->type == MOD_TYPE_SOCKET)
      continue;
    if (module->type == MOD_TYPE_DB)
      continue;
        
    __module_close(module); //result here is that the module is completely gone.
                            //and remove from our list.
  }
  
  /* make sure we purge socketnegine/database module last - this 
   * looks like were looping twice but there should
   * only be one/two entries in the list at this point
   */
  DLINK_FOREACH_SAFE (dl,tdl, modules.head)
  {
    module = dl->data;                
    __module_close(module); 
  }
}

/********************************************/
/**
 **      Module Loader Event Hooks
 **
 **/
 
 
int mod_event_shutdown(int ac, void* args)
{
  modules_purge();
  return 0;
}

int mod_event_rehash(int ac, void* args)
{
  run_mod_que(MOD_QUEUE_PRIO_PRE);
  run_mod_que(MOD_QUEUE_PRIO_STD);
  run_mod_que(MOD_QUEUE_PRIO_POST);
  return 0;
}
