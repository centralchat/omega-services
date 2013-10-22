#ifndef ____SOCKET_H__
#define ____SOCKET_H__


#define SOCKET_NEW      0x00000000
#define SOCK_CONNECTING	0x00000001
#define SOCK_UPLINK		0x00000002
#define SOCK_LISTEN		0x00000004
#define SOCK_CTRL		0x00000008
#define SOCK_WRITE		0x00000010
#define SOCK_READ		0x00000020
#define SOCK_NOREAD		0x00000040
#define SOCK_DEAD		0x00000080


#define SOCK_CLUSTER	0x00000100 //Reserve for when we do clustering


#define socket_is_connecting(s) ((s)->flags & SOCK_CONNECTING)
#define socket_is_read(s)  ((s)->flags & SOCK_READ)
#define socket_is_write(s) ((s)->flags & SOCK_WRITE)
#define socket_is_listen(s) ((s)->flags & SOCK_LISTEN)
#define socket_is_dead(s)  ((s)->flags & SOCK_DEAD)

#define socket_is_fdset(s, set) FD_ISSET ((s)->sd, &(set))
#define socket_fdset(s, set) FD_SET ((s)->sd, &(set))

int max_sockets;


/****************************************************/
/**
 * MessageBuffer - Since we cant always guarantee
 * we get all our info in a single packet, lets 
 * buffer
 */
 

typedef struct
{
	char   message[MAXLEN];
	size_t length;
	int    completed;
	
} MessageBuffer;


/****************************************************/
/**
 * Socket
 */

typedef struct socket_
{
	time_t  connected_at;
	
	int	sd;
	int timeout;
	int flags;
	int lines;
 
	unsigned int buffer_len;
	char	buffer[MAXLEN + 1];	

	char	name[512];

	struct  sockaddr_in	*sa;   

	dlink_list	msg_buffer;
	//Callbacks
	void	(*read_callback)  (struct socket_ *);
	void	(*write_callback) (void *);
    void    (*error_callback) (struct socket_ *);

} Socket;


E Socket * socket_new       ();
E int      socket_listen    (Socket *);
E void     socket_dead      (Socket *);
E void     socket_free      (Socket *);
E void     socket_remove    (Socket *);
E int      socket_write     (Socket *, char *, ...);
E int      socket_read      (Socket * );

E void socket_empty_callback(void);


/****************************************************/
/**
 * Socket callback macros
 */


#define socket_read_callback(s)     (s)->read_callback  ? (s)->read_callback((s))    : socket_empty_callback()
#define socket_write_callback(s)    (s)->write_callback ? (s)->write_callback((s))   : socket_empty_callback()
#define socket_error_callback(s)    (s)->write_callback ? (s)->error_callback((s))   : socket_empty_callback()

#define  socket_set_read_callback(s, fnct)  (s)->read_callback  = fnct
#define socket_set_write_callback(s, fnct)  (s)->write_callback = fnct
#define socket_set_error_callback(s, fnct)	(s)->error_callback = fnct


#define socket_set_callbacks(s, read, write, error) \
 	(s)->read_callback  = read;  \
 	(s)->write_callback = write; \
 	(s)->error_callback = error


/****************************************************/
/**
 * Socket list functions
 */


#define socket_find socket_find_sd

int      socket_delfrom_list (Socket *);
int      socket_addto_list   (Socket *);
Socket * socket_find_by_sd   (int);
Socket * socket_find_by_name (char * name);
void     socket_purge_all    (void);
void     socket_purge_dead   (void);
char   * socket_getline      (Socket *s);

dlink_list sockets;

#endif