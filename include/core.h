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
 *    $Id: core.h 2366 2012-02-27 05:38:02Z twitch $
 */

#ifndef _CORE_H_
#define _CORE_H_

/* Paths */

#define DPATH	PREFIX
#define EPATH	ETC_DIR
#define MPATH	MODULE_DIR
#define CPATH   MODULE_DIR "/core/"
#define PPATH	MODULE_DIR "/protocol/"
#define LPATH   "/logs/"
#define CONTRIB_PATH MODULE_DIR "/contrib/"
#define SPATH	PREFIX "/bin/security"
#define DBPATH  MPATH "/db/"

#define CONFIG_NAME EPATH "/security.cfg"

#define TMP_DIR PREFIX "/var/tmp"
#define CLIENT_MOD_DIR MODULE_DIR "/client/"

#define MAXPATH	255
#define HASH(x,y) hash_safe(x,y)

/**
  * 	Free work around and sanity checker
  */


#define MyFree(x) 		\
do {	 			\
	assert((x) != NULL);	\
				\
	if((x) != NULL) 	\
	{			\
	     free(x);   	\
	     x = NULL;		\
	}			\
} while (0)


/**
 * Use built in debug commands. These commands
 * bypass the command system and are only for
 * debugging the CORE - There is no documentation
 * on these commands...
 */

#ifdef DEBUG_MODE
# define VER_DEBUG
# define USE_DEBUG_CMDS
#else
# undef VER_DEBUG
# undef USE_DEBUG_CMDS
#endif



/* Soft asserts: This is borrowed from Ratbox/KC-IRCd */
#ifdef SOFT_ASSERT

# ifdef __GNUC__
#  define s_assert(expr)  do                              \
            if(!(expr)) {                           \
				alog(4, "file: %s line: %d (%s): Assertion failed: (%s)", \
				__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr); \
            }                               \
            while(0)
# else
#  define s_assert(expr)  do                              \
            if(!(expr)) {                           \
                alog(4, "file: %s line: %d: Assertion failed: (%s)", \
                __FILE__, __LINE__, #expr); \
            }                               \
            while(0)
# endif
#else
#	define s_assert(expr)  assert(expr)
#endif


#define MAXPARA   15
#define MAXSTRING 18
#define NICKMAX   30
#define MAXSERV   63       // The ircd only allows hosts to be 63 characters

pid_t pid;

int debug;
int backtrc;
int nofork;

int connect_attempts;

int introduce_attempts;
time_t last_attempt;

int recvbytes;  //amount of information received
int sentbytes;  //amount of sent information

time_t  starttime;

struct memcounts {
	long	userssize;
	long	users;

	long	chanusize;
	long	chanucnt;

	long	chansize;
	long	channels;

	long	servcnt;
	long	servsize;

	long	sendqcnt;
	long	sendqsize;

    long    eventscnt;
    long    eventssize;

    long    msgbufsize;
    long    msgbufcnt;

    long    buflinesize;
    long    buflinecnt;

	long	socketcnt;
	long	socketsize;
} memcounts;

/********************************/
/**
 *       CONFIG SETTINGS
 */

 
typedef struct config_ {
    char    servername[MAXSERV + 2]; //Our server name :

    char    uplink[MAXSERV + 2];     //Uplink either aa.bb.cc.dd ip or real host

    int     port;                    //port we are connecting to must be numeric

    char    pass[32];

    char    numeric[10];

    char    desc[80];

    char    sid[4];

    char    protocol[32];

    char    conf_name[512];

    char    pidfile[MAXPATH];

    int     privmsg;

    struct hostent	*local_host;

    char	local_ip[MAXSERV];

	char	network[256];

    int     operonly;
    int     gnutls;

    char    umode[30];

} Conf;

Conf CfgSettings;

//we should be using Protocol type for
//holding this information no sense in holding it twice? -- Twitch

struct ProtocolInfo {
	char name[50];
	int	ts;
	int	ts6;
} ProtocolInfo;


char logchan[32]; //max chan for now is assumed to be 30 + 2 bytes padding.
char datadir[MAXPATH];

/********************************/
/**
 *        INT WORK AROUND
 */


typedef int16_t    int16;
typedef u_int16_t  uint16;
typedef int32_t    int32;
typedef u_int32_t  uint32;

/********************************/
/**
 *          LINK STRUCT
 */


typedef struct links_ {
    time_t linktime; //time the link was created

    char    name[MAXSERV + 2]; //name of the server

    char    desc[512];

    char    sid[15]; // UID of the linking server

    char    linked[MAXSERV + 2]; //The server this is linked to in squits :P

    int     num; //number used if we use token commands :/ and receive @# instead of SERVER

    int     eos; //have we got the EOS from this server?

    time_t	lastpong;

    struct links_ *next,*prev;

    struct links_  *leafs;

    struct links_   *uplink;
} Link;

typedef struct cmdhash_ CmdHash;


// XXX - Depreciate the Uplink which
//holds a pointer to our link/so we
//don't have to walk the list each time.

Link *Uplink;
Link *Links;
Link *Me;


/******************************************/
/**
 *              CMODE STRUCT
 */

typedef struct cmode_info {

	uint32 mode;

	uint32 flags;

} CMode;

/******************************************/
/**
 *               SYNC STATES
 */


#define ERR_STATE -3
#define QUIT      -2
#define SHUTDOWN  -1
#define STARTUP    0
#define CONNECTING 1
#define BURSTING   2
#define RUNNING    3


//depreciated server states - not quite removed from the core 100%
#define IS_CON 3
#define NOT_CON 0

#define serv_state(state) (((state) != sync_state)? 0 : 1)


int sync_state; //Holds the state of the server.

/******************************************/
/**
 *                  TIME
 */


struct tm *curdate;   //current date

struct tm *lastdate;  //If our date changed take a sample of the last date :)
                      //albeit i don't know if this is really needed

time_t ServerTime;   //current server time in time_t format :)


int logcmds;
int logchannels;

/******************************************/
/**  
 *             CORE STRUCT
 */
 
 
/*
 * This structure should hopefully allow for modules to easily access runtime options - and extend our Core API
 * a bit more... :)
 */
 
typedef struct core_ {

	time_t age;
	
	int state; //what state is our server in
	
	Link *me; //our link structure
	
	void (*exit)(int); //exit handler
	
	void (*rehash)(int); //rehash handler
	
	void (*error)(int); //error handler
	
	void (*log)(int, char*, ...); //log handler
	
} Core; 


Core* Omega;


Core* Core_Construct	();

/******************************************/
/**
 *              PROTOTYPES
 */

void init_core();

int check_for_pid();
void Exit(int); //exit :)
void Rehash(int); //rehash

void OneTimeAround  ();
void SaveAll        (int);
void SetTime        (); //set server time structures to the current time :)

void init_lists (void);
int session_count;
size_t session_size;
int skip_banner;
#endif
