#ifndef __CORE_H__
#define __CORE_H__


enum SYNC_STATES { 
	SYNC_STATES_START,
		SYNC_STATE_STARTUP,
		SYNC_STATE_NORMAL,
		SYNC_STATE_SHUTDOWN,
	SYNC_STATES_END
};

#define R_PATHLEN 1024

#define ENDBLOCK "\r\n\r\n"

#define INFO_ATTRIBUTES \
	char file[MAXPATH]; \
	char func[MAXFUNC]; \
	int  lineno


/********************************/
/**
 *       Core/Config info
 */
 

struct _core { 
	time_t started_at;
	time_t time;

	int debug;
	int	nofork;
	int generate_backtrace;
	int skeleton;

	dlink_list databases;

	struct {
		struct  hostent	*local_host;
	  char    pid_file[MAXPATH];
		char	  local_ip[MAXHOST];
		int     port;
		char    config_file[MAXPATH];

		//Should this really go here?
		char    protocol[MAXFUNC];

		struct { 
			char name[MAXHOST];
			char sid[10];
			char numeric[10];
		} server;

		struct { 
			char name[MAXHOST];
			char network[32];
			char pass[64];
			int  port;
		} link;

	} settings;


	struct { 
		char **env;
		char **argv;
		int  argc;
	} cmdline;

	Socket * console;
	Socket * socket;

	server_t * uplink;
	server_t * server;
	
} core;


typedef struct {
	union { 
		server_t *server;
		void     *ptr;
	} source;

	int  argc;
	char **argv;

	void *ptr; //used for passing random information to the method
} args_t;


#define ARGS(arg_struct, src, ac, av) 		            \
	(arg_struct) = NULL; 													      \
	if (((arg_struct) = alloca(sizeof(args_t))))        \
	{																						        \
		if ((av))  (arg_struct)->argv   = (char **)(av);  \
		else (arg_struct)->argv         = NULL;		  			\
		if ((src)) (arg_struct)->source.ptr = (src);		  \
		else (arg_struct)->source.ptr   = NULL;						\
		(arg_struct)->argc   = (ac);						          \
	}																						        \
	(arg_struct)

int debug;

E int sync_state;
E void core_exit          (int);
E int  core_reload        (int);
E void core_init          (void);
E void core_run           (void);
E void core_cleanup       (void);
E void core_once_around   (void);
E int  core_connect_uplink(void);
E void core_parse_opts    (int, char **, char **);
E void daemonize(void);


#endif
