/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
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
 *    $Id: main.c 2394M 2013-08-15 03:58:37Z (local) $
 */


#include "stdinc.h"
#include "server.h"
#include "sig.h"
#include "parser.h"

static void showHelp ();

extern int uselogchan;

/***************************************/

void Run        (void);
void DaemonSeed (void);
void Banner     (void);

void Load_Dbs (void);

ServCmds* servcmds_list = NULL;
UserCmds* usercmds_list = NULL;

Module* module_list        = NULL;

static void AtExit(void);
void do_restart(void);

extern void psr_HandleEvent(Socket* s,int et);

//copy of our arguments and environment
char **my_env,**my_argv;

/***************************************/

int main(int argc, char **argv, char **env)
{
	int c, ret;
	my_argv	= argv;
        char *ce;

    uselogchan  = 1;
    starttime   = time(NULL);
    skip_banner = 0;

#if defined(RLIMIT_CORE) && defined(HAVE_SYS_RESOURCE_H)
    // Make sure we can dump a core
    struct rlimit   rlim;
    ret = getrlimit (RLIMIT_CORE, &rlim);

    if (ret == 0)
    {
        if (rlim.rlim_cur == 0)
        {
            //fprintf (stderr, "Core limit 0, attempting to set to %d\n", rlim.rlim_max);

            rlim.rlim_cur    = rlim.rlim_max;

            setrlimit (RLIMIT_CORE, &rlim);
        }
    }
#endif

#ifdef HAVE_GNUTLS
    gnutls_global_init ();

    gnutls_certificate_allocate_credentials (&tls_x509_cred);
#endif

		SetTime();

        sprintf (CfgSettings.conf_name, "%s/%s", EPATH, "security.cfg");

        my_env = env;

        while ((c = getopt (argc, argv, "xrshbvntd:c:l:")) != -1)
        {
            switch (c)
            {
                case 'x':
			skip_banner = 1;
			break; 
				case 'h':
					showHelp ();
                    break;
                case 'v':
                        printf("Version: \033[1;31m%s %s-%s\033[0m\n",PACKAGE_NAME,PACKAGE_VERSION,VERSION_STRING);
                        exit(0);
                case 's':
                        nomodules = 1;
                        break;
                case 'd':
						if (optarg)	{
							debug	= atoi(optarg);
							if ((debug < 1) || (debug > 3))
							{
								fprintf (stderr, "Invalid value for -d (%d)\n", debug);
								exit (0);
							}
						}
                        break;
                case 'n':
                        nofork = 1;
                        break;
                case 'c':
						if (optarg) {
	                        if ((optarg[strlen(optarg) - 4] == '.') && optarg[strlen(optarg) - 3] == 'c') {
                                if ((*optarg == '/') || (*optarg == '.'))
                                    strlcpy (CfgSettings.conf_name, optarg, sizeof(CfgSettings.conf_name));
                                else
	                                sprintf (CfgSettings.conf_name, "%s/%s", EPATH, optarg);
                            }
							else
								fprintf(stderr,"Invalid config file specified with -c option skipping.\n");
						}
                        break;
                case 'l':
                        if (optarg)
                            memcpy(logchan,optarg,sizeof(logchan));
                        break;
                case 't':
			config_test();
			exit(0);
            }
        }


        //check to see if were running under root and bail
        if ((getuid() == 0) && (getgid() == 0))
        {
            printf("Error: You are currently trying to run this program as root\n");
            printf("This program does not require root privileges ...\n");
            printf(" \n");
            printf("Running an application as root that does not require root privileges\n");
            printf("can and may have undesirable side effects. \n");
            printf("    We strongly caution against this.       \n");
            exit(0);
        }

        if (chdir (DPATH))
        {
              fprintf (stderr, "Unable to chdir to %s: %s\n", DPATH, strerror(errno));
              exit (EXIT_FAILURE);
        }


		Omega = Core_Construct(); //initialize our core class.
		
        init_lists ();
		InitDefaults();
	
        //we should work on making this more detailed in its error throws
        if (!config_load (CfgSettings.conf_name)) {
               fprintf(stderr, "Unable to read config file.\n");
               exit(0);
        } 
		VerifyConf();
		 

        DaemonSeed();

        atexit(AtExit);
	 
        Run();

        //we returned from Run() Some how :/ i shit you not.. we need to exit ;( *tear*
            Exit(0);
        return 0;
}


/******************************************/

void Banner(void)
{
    
	if (skip_banner)
		return;
	printf("\n");
    printf("Omega Security Services Daemon\n");
    printf(" Version: \033[1;31m%s %s\033[0m\n",PACKAGE_VERSION,VERSION_FULL);
    printf(" Compiled: \033[1;31m%s\033[0m\n",COMPILE_DATE);
	printf(" On: \033[1;31m%s\033[0m\n", SYSUNAME_SHORT);
    printf(" Process ID: \033[1;31m%lu\033[0m\n",(unsigned long) getpid());
    printf("\r\n");
    printf(" Developers: \033[1;32m Twitch Jer\033[0m\n");
    printf("\r\n");
}



/******************************************/

void DaemonSeed(void)
{
    FILE *pidfd;
	int our_pid;


        if (nofork != 1)
        {


            //check to see if were currently running :)
            if ((our_pid = check_for_pid()) > 0) {
			printf("Omega is currently running [Pid: %d]\n", our_pid);
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


            pidfd = fopen(CfgSettings.pidfile,"w+");
            fprintf(pidfd,"%lu",(unsigned long) getpid());
            fflush(pidfd);
            fclose(pidfd);

        }
}


/***********************************/
/**
 * Move this to its own function
 *
 * return 0 if there is no pid or -1 if there is no file else
 * return the current active pid.
 */

int check_for_pid() {
  	    FILE *pidfd;
	    int our_pid;
	    int value, ret;
       	char tmp[20];

 	    if ((pidfd = fopen(CfgSettings.pidfile,"r")))
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
					unlink (CfgSettings.pidfile);
				 	return 0;
				case 0:
					return our_pid;
		
			}
	    } 
	    return -1; //no pid file.
}

/******************************************/
//there has to be a better way to do this :/
void Run(void)
{
  
    char *ce; 
    SetSig       ();
    Banner       ();

        if ((ce = (char *)get_config_entry("?omega", "version")))
        {
			if (CONFIG_VERSION != atoi(ce))
			{
				fprintf(stderr, "\033[1;31mCRITICAL\033[0m: Invalid configuration file version. [Given: %s][Required: %d]", ce, CONFIG_VERSION); 
				Exit(0);	
			}	
	} else { 
		fprintf(stderr, "\033[1;31mERROR\033[0m: Unable to determine the configuration file version. Please make sure you have all <?omega> intact\n");
		fprintf(stderr, "Assuming configuration version \033[1;31m%d\033[0m\n\n", CONFIG_VERSION);
	} 

#ifdef HAVE_SETPROCTITLE
    setproctitle("%s", CfgSettings.servername);
#endif

    open_log  ();
    init_uid ();
	
    AddEventEx ("House Keeping", 900, EVENT_UNLIMITED, ev_housekeeping);
 
    AddEvent ("BURST", burst_local_users);
    AddEvent ("BURST", burst_local_servers);
    AddEvent ("CONNECT", introduce_users);
	
    if ((Omega->me = new_serv(NULL, CfgSettings.servername)))
    {
        strlcpy (Omega->me->sid, CfgSettings.sid, sizeof(Omega->me->sid));
        Omega->me->eos = 1; //mark us as already EOS'ed
    }
    else
    {
		fprintf (stderr, "Unable to create our server entry\n");
		exit (0);
    }

    /*
     * Initialize our core for running
     */

	database_init (); 
    protocol_init ();
	
    servsock    	 = add_new_socket ((void*) psr_HandleEvent, NULL);
    servsock->flags	|= SOCK_UPLINK;
	strlcpy (servsock->name, "Uplink", sizeof(servsock->name));

#ifdef HAVE_GNUTLS
        servsock->tls_enabled   = get_config_bool("link", "gnutls", 0);
#endif
	
    psr_init      ();
	init_access();

	init_modules();
	introduce_users (0, NULL);

    /*
     * Connect to the IRCd if we don't connect Add an event to connect...
     */

    printf ("\r\n");
    printf ("Connecting to \033[1;32m%s\033[0m port \033[1;32m%d\033[0m\033[0m\n", CfgSettings.uplink, CfgSettings.port);
    if (Connect (servsock, CfgSettings.uplink, CfgSettings.local_ip, CfgSettings.port))
        ircd_connect ();
    else
        AddEventEx ("Connect Uplink", 60, 3, ev_connectuplink);

    /*
     * Okay begin
     */

    OneTimeAround   ();

    return;
}

/******************************************/

void OneTimeAround()
{
	  int status = 0;
		
       while (1) 
       {
	      /*
	       * run these in the core its self...
	       * i hated the fact the select engine 
	       * ran these.
	       */
	      SetTime ();
	      RunTimedEvents ();

          run_mod_que(MOD_LOAD_POST);

	      //allow for better core control over the socketengine!
		  status = receive(NULL);
	      switch (status)
	      {
				case SOCK_ERR_OK:
					break;
				default:
					alog(LOG_SOCKET,"Received returned: %d", status);
					break;
	      }
	}
	return;       // We should only reach this if we are shutting down
}

/******************************************/

void SetTime() {
	time_t t;
    struct tm *tmp;

    time(&t);
    tmp = localtime(&t);

	//record the server time as were passing through
	ServerTime = (unsigned long) time(NULL);

	//check to see if its a new date :) or
	//if we just start set our last date to
	//our current date :)

	if ((curdate) && (tmp))
	{
		if (curdate->tm_mday != tmp->tm_mday)
			lastdate = curdate;
	}
	curdate = tmp;
 }

/******************************************/
/**
  * AtExit() Called by atexit when program terminates
  * this insures we have proper cleanup :)
  *
  * @return void - must return void
  */

static void AtExit(void)
{

    sync_state = SHUTDOWN;
	
	Event("UPDATEDB", 0, NULL);
	Event("SHUTDOWN", 0, NULL);

	
	
    uplink_cleanup("Exiting",1);

    unlink (CfgSettings.pidfile);

	socket_cleanup(); //destroy all outgoing socket stuff.
		
    DestAccList();
	
#ifdef HAVE_PTHREAD
    dest_all_threads();
#endif
	
#ifdef USE_DMALLOC
	dmalloc_shutdown();
#endif

#ifdef HAVE_GNUTLS
    gnutls_global_deinit ();
#endif
	
	destroy_event_list();
	
	database_deinit();
	close_log();
    
	if (Omega)
		free(Omega);
	
	return;
}

/******************************************/
/**
 *  Load_Dbs()
 *
 */

void Load_Dbs(void)
{
	return;
}

/******************************************/
/**
 * Our exit function - since everything is pretty much handled
 * in AtExit we are just calling exit(0) for the time being
 */

void Exit(int status)
{
	
	exit(status);
}

/******************************************/

void Rehash(int status)
{
	sendto_logchan("Reloading configuration files");
		
       destroy_config_tree();
       config_load (CfgSettings.conf_name); //load the config

	Event("REHASH", 0, NULL);
		
}



/******************************************/

void do_restart()
{
	Event("SHUTDOWN", 0, NULL);
	uplink_cleanup ("Restarting", 1);
	unlink (CfgSettings.pidfile);
	execv(SPATH, my_argv);
	exit(0);
	return;
}



/******************************************/

void init_lists	()
{
	userlist.head	= userlist.tail	= NULL;
	servlist.head	= servlist.tail	= NULL;
	events.head		= events.tail   = NULL;
	sockets.head	= sockets.tail	= NULL;
	accesslist.head	= accesslist.tail	= NULL;
	sockevents.head	= sockevents.tail	= NULL;
	access_flags.head = access_flags.tail = NULL;
	connected_sockets.head	= connected_sockets.tail			= NULL;
	return;
}


/******************************************/

static void showHelp ()
{
	printf ("Available Arguments\n");
	printf ("\n\n");

	printf ("Runtime options:\n");
	printf ("-d[1|2|3]    run in debug mode\n");
	printf ("-n           Do not fork into the background.\n");
	printf ("-b           Generate backtraces internally.\n");
	printf ("-r           Reload configuration and modules.\n");
	printf ("-s           Run in skeleton mode, Additional modules disabled.\n");
	printf ("-v           Display version information\n");
	printf ("-h           Display this help\n");
	printf ("\n\n");

	printf ("Configuration options:\n");
	printf ("-c [config name]    Use the named configuration.\n");
	printf ("-l [log channel]    Use the specified log channel.\n");

	exit (0);
}

/* vim: set tabstop=4 shiftwidth=4 expandtab */

