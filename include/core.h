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

/********************************/
/**
 *       CONFIG SETTINGS
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
	} settings;

	struct { 
		char **env;
		char **argv;
		int  argc;
	} cmdline;

} core;

int debug;
int sync_state;

void core_init        (void);
void core_run         (void);
void core_exit        (int);
void core_cleanup     (void);
void core_once_around (void);
void core_parse_opts  (int, char **, char **);

#endif