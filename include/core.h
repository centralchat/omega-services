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

	dlink_list databases;

	struct {
		struct  hostent	*local_host;
	    char    pid_file[MAXPATH];
		char	local_ip[MAXHOST];
		int     port;
		char    config_file[MAXPATH];
		char    con_host[MAXHOST];
		int     con_port;
	} settings;

	struct { 
		char **env;
		char **argv;
		int  argc;
	} cmdline;

	Socket * socket;

} core;

int debug;

E int sync_state;
E void core_init          (void);
E void core_run           (void);
E void core_exit          (int);
E void core_cleanup       (void);
E void core_once_around   (void);
E void core_parse_opts    (int, char **, char **);
E int  core_reload        (void);
E int  core_connect_uplink(void);


#endif