/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008 Omega Dev Team
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
 *    $Id: sig.c 2252 2012-01-22 09:21:23Z twitch $
 */


#include "stdinc.h"
#include "server.h"

extern User* user_list;
extern Link* serv_list;
extern void do_restart(void);

void Users  (int);
void Info   (int);
void GenCore(int);
void SigHandler(int);

void SetSig()
{
	signal(SIGINT,	SigHandler);
	signal(SIGTERM, SigHandler);
    signal(SIGUSR2, SigHandler);
    signal(SIGHUP,	SigHandler);
    signal(SIGUSR1, SigHandler);
	
	signal(SIGPIPE, SigHandler);
	signal(SIGTRAP, SIG_IGN);

        if (backtrc == 1)
        {
        	signal(SIGABRT, GenCore);
        	signal(SIGSEGV,GenCore);
            signal(SIGILL,GenCore);
        }
}

void SigHandler(int sig)
{
	Access* ca;
        dlink_node *dl,*hdl;
        char* host;
	if (sig == SIGPIPE)
		return;
	
	alog (LOG_INFO, "Caught signal from console - %d",sig);
	switch (sig)
	{
		case SIGHUP:
			Rehash(0);
			break;
		case SIGPIPE:
			break;
		case SIGINT:
		case SIGTERM:
			Exit(0);
			break;
		case SIGUSR1:
		printf("Access List: \n");	
  DLINK_FOREACH(dl,accesslist.head)
                {
                         ca = dl->data;
                        DLINK_FOREACH(hdl,ca->hosts.head)
                        {
                                host = hdl->data;
                                 	fprintf(stderr,":O %s %s %s\n",ca->name,
                                                host,ca->flags);

                        }

                }


			break;	
		case SIGUSR2:
			do_restart();
			break;
	}

}


void GenCore(int sig)
{

#ifdef HAVE_BACKTRACE
	FILE* f;
        void *info[10];
        size_t size, i;
        char **string;

        alog(LOG_BACKTRACE,"---------------------------------");
        alog(LOG_BACKTRACE,"Error detected (%d)",sig);
        alog(LOG_BACKTRACE,"Attempting to display a backtrace");
        alog(LOG_BACKTRACE,"---------------------------------");

        size = backtrace (info, 10);
        string = backtrace_symbols (info, size);

        for (i = 0; i < size; i++)
            alog(LOG_BACKTRACE,"%s",string[i]);

        free(string);

        alog(LOG_BACKTRACE,"---------------------------------");
#endif
        abort();
}
