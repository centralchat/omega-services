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
 

void add_se_fnct_receive(int  (*fnct)());
void add_se_fnct_cleanup(void (*fnct)());
void add_se_fnct_startup(int  (*fnct)());
void del_se_fnct_receive(int  (*fnct)());
void del_se_fnct_cleanup(void (*fnct)());
void del_se_fnct_startup(int  (*fnct)());

int  se_startup ();
void se_cleanup ();
int  se_receive ();
int  se_accept  (Socket *);
int  se_read    (Socket *);
int  se_listen  (Socket *, char *, int);
int  se_connect (Socket *, char *, char *, int);




#ifdef HAVE_GETADDRINFO
	struct addrinfo * gethostinfo(char const *, int);
#endif 

#endif
