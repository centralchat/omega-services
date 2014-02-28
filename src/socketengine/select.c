#include "modinclude.h"
#include "stdinclude.h"
#include "appinclude.h"


static int  select_load();
static void select_unload();
static int  select_receive();
static void select_cleanup();


MODHEADER("Select Engine", 
	"Omega Development",
	"0.0.1", 
	MAKE_ABI(0,0,1), 
	select_load, 
	select_unload
);


fd_set wfdset,
	rfdset,
	errfdset;


static int select_load() 
{
	add_se_fnct_receive(select_receive);
	add_se_fnct_cleanup(select_cleanup);
	return MOD_CONT;
}

static void select_unload()
{
	del_se_fnct_receive(select_receive);
	del_se_fnct_cleanup(select_cleanup);
	return;
}

static void select_cleanup() {
	socket_purge_all();
}

static void select_build_fdsets() 
{
	dlink_node *dl, *tdl;
	socket_t * s;

	FD_ZERO(&wfdset);
	FD_ZERO(&rfdset);
	FD_ZERO(&errfdset);

	DLINK_FOREACH_SAFE (dl, tdl, sockets.head)
	{

		s = dl->data;
		
		if (socket_is_dead(s))
			continue;
		else if (socket_is_listen(s))
			FD_SET (s->sd, &rfdset);		
		else if (socket_is_connecting(s) || socket_is_write(s))
			FD_SET (s->sd, &wfdset);
		else if (socket_is_read(s))
			FD_SET (s->sd, &rfdset);

		alog(LOG_DEBUG3, "socket_t is in: %s (Fd: %d)", socket_is_read(s) ? "Read" : "Write", s->sd);
	}
}

static int select_receive(void) { 
	int	rc;
	dlink_node *dl, *tdl;
	struct timeval  timeout;
	socket_t * tmp_sock;

	socket_t * s;

	socket_purge_dead();

	select_build_fdsets();
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 250 * 1000;
                
  log_message(LOG_DEBUG3, "Currently %d sockets in the list", sockets.count);
    
	rc = select (max_sockets + 1, &rfdset, &wfdset, NULL, &timeout);
	if (rc > 0) // This indicates we have fd's ready for read/write
	{
		DLINK_FOREACH_SAFE (dl, tdl, sockets.head)
		{
			s = dl->data;
			if (FD_ISSET (s->sd, &rfdset))
			{	
				if (!socket_is_listen(s))
				{

					int bread = socket_read(s);
					if (bread <= -1)
					{
						socket_error_callback(s);
						socket_remove(s);
						continue;
					}				
					log_message(LOG_DEBUG3, "Calling read for socket: %d", s->sd);	
					socket_read_callback(s);
				} 
				else 
				{ 
					tmp_sock        = socket_new();
					tmp_sock->sd    = se_accept(s);
					tmp_sock->flags = SOCK_READ;

					socket_accept_callback(s);

					socket_addto_list(tmp_sock);
					log_message(LOG_SOCKET, "New connection: %d", tmp_sock->sd);
					continue;
				}
			}
			if (FD_ISSET (s->sd, &wfdset))
			{
				tmp_sock->flags = SOCK_READ;
			}
		}
	}

	return 1;
}
