#ifndef ___SERVER_H___
#define ___SERVER_H___


typedef struct _server {

    time_t linktime; //time the link was created

    char    name[MAXHOST + 2]; //name of the server

    char    desc[512];

    char    sid[15]; // UID of the linking server

    char    linked[MAXHOST + 2]; //The server this is linked to in squits :P

    int     num; //number used if we use token commands :/ and receive @# instead of SERVER

    int     eos; //have we got the EOS from this server?

    time_t	lastpong;

    dlink_list leafs;

    struct _server *uplink;

} server_t;

typedef struct _server Link; //alias for some backward compat

dlink_list servers;

server_t * server_new();
void       server_free(server_t *);

#define server_find server_findby_name

server_t * server_findby_name ();
server_t * server_findby_sid  ();

void server_addto_list(server_t *);

#endif
