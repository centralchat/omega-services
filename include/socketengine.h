#ifndef __SOCKETENGINE_H__
#define __SOCKETENGINE_H__

//#define MAXLEN 16384
//#define MAXLEN 4096

#define MAXLEN 2048
#define MAXREADBUFFER MAXLEN*120+5

/****************************************************/
/**
 * SocketEngine API
 */

struct { 
	int  (*startup)();
	void (*cleanup)();
	int  (*receive)();
	Thread * thread;
} SocketEngine;

E void add_se_fnct_receive (int  (*fnct)());
E void add_se_fnct_startup (int  (*fnct)());
E void del_se_fnct_receive (int  (*fnct)());
E void del_se_fnct_startup (int  (*fnct)());

E void add_se_fnct_cleanup (void (*fnct)());
E void del_se_fnct_cleanup (void (*fnct)());
 
E int  se_startup ();
E void se_cleanup ();
E int  se_receive ();

E void se_init         (void);
E int  se_accept       (Socket *);
E int  se_read         (Socket *);
E int  se_unix_connect (Socket *, char *);
E int  se_listen       (Socket *, char *, int);
E int  se_connect      (Socket *, char *, char *, int);

#ifdef HAVE_GETADDRINFO
	struct addrinfo * gethostinfo(char const *, int);
#endif

#endif
