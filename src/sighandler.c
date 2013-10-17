#include "stdinclude.h"
#include "appinclude.h"

void generate_backtrace(int);
void sighandler(int);

void sighandler_init()
{
	signal(SIGINT,	sighandler);
	signal(SIGTERM, sighandler);
    signal(SIGUSR2, sighandler);
    signal(SIGHUP,	sighandler);
    signal(SIGUSR1, sighandler);
    signal(SIGPIPE, sighandler);
	signal(SIGTRAP, SIG_IGN);
}

void sighandler(int sig)
{
	//SIGPIPE on a socket is safe to ignore
	if (sig == SIGPIPE)
		return;
	
	log_message (LOG_INFO, "Caught signal from console - %d",sig);
	switch (sig)
	{
		case SIGHUP:
			//core_rehash(0);
			break;
		case SIGPIPE:
			break;
		case SIGINT:
		case SIGTERM:
			core_exit(sig);
			break;
		case SIGUSR1:
			break;	
		case SIGUSR2:
			//core_restart();
			break;
	}

}
