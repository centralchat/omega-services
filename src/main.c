#include "setup.h"
#include "stdinclude.h"
#include "appinclude.h"

static inline void core_startup_banner(void)
{
  printf("\n");
  printf("Omega Security Services Daemon\n");
  printf(" Build: \033[1;31m%s\033[0m\n ", VERSION_STRING_FULL);
  printf(" Compiled: \033[1;31m%s\033[0m\n", COMPILE_DATE);
  printf(" On: \033[1;31m%s\033[0m\n", SYSUNAME_SHORT);
  printf(" Process ID: \033[1;31m%lu\033[0m\n",(unsigned long) getpid());
  printf("\r\n");
  printf(" Developers: \033[1;32m Twitch Jer\033[0m\n");
  printf("\r\n");
}


int check_for_pid() {
	FILE *pidfd;
	int our_pid;
	int value = 0;
	int ret = 0;
	char tmp[20];

  if ((pidfd = fopen(core.settings.pid_file,"r")))
  {

		memset(tmp,0,sizeof(tmp));
		ret = fread(tmp,1,sizeof(tmp),pidfd);
   	fclose(pidfd);

  	our_pid = atoi(tmp);
		value = kill((pid_t) our_pid, 0);
	
		switch (value)
		{
			case ESRCH:
				//destroy the pid file
				unlink (core.settings.pid_file);
			 	return 0;
			case 0:
				return our_pid;	
		}
	} 
  return -1; //no pid file.
}


void daemonize(void)
{
  FILE *pidfd;
	int our_pid;
	int pid;

  if (!core.nofork)
  {
    //check to see if were currently running :)
    if ((our_pid = check_for_pid()) > 0) {
			printf("Application is currently running [Pid: %d]\n", our_pid);
			exit(1);
	  }
    //here we go forking our process
    if ((pid = fork ()) < 0) {
         printf("ERROR: Unable to fork process\n");
         exit(1);
    }  else if (pid > 0) {
         sleep(2);
         exit(0);
    }
    setsid ();
    pidfd = fopen(core.settings.pid_file,"w+");
    fprintf(pidfd,"%lu",(unsigned long) getpid());
    fflush(pidfd);
    fclose(pidfd);
  }
}

#ifndef TESTFRAME_WORK

int main(int argc, char **argv, char **env)
{
   

		//Bootstrap our core here prior to initing it.
    load_config_defaults();
    core_parse_opts(argc, argv, env);

     //db_connect("localhost", "root", "root");
    core_startup_banner();

    core_init();
    core_run();
    return 0;
}

#endif
