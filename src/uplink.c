#include "stdinclude.h"
#include "appinclude.h"


/*********************************************************/


int uplink_connect()
{
    int ret = 0;

    core.socket = socket_new();
    core.socket->flags |= SOCK_UPLINK;
    // socket_set_callbacks(core.socket, uplink_read_callback, NULL, NULL);

    core.socket->read_callback = uplink_read_callback;

    log_message(LOG_INFO, "Connecting to: %s:%d", core.settings.link.name, core.settings.link.port);
    ret = se_connect(core.socket, core.settings.link.name, 
                core.settings.local_ip, core.settings.link.port);

    if (ret <= 0)
    {
        log_message(LOG_ERROR,"Unable to connect to %s:%d", core.settings.link.name, 
            core.settings.link.port);        
        socket_free(core.socket);
    }   
    else 
    {
        if (socket_addto_list(core.socket))
            log_message(LOG_DEBUG2,"Connected: %d", core.socket->sd);

        event_dispatch("UPLINK", NULL);
    }
    return ret;
}


/*********************************************************/
/**
 * Not sure if i like this methodology theoretically
 * i think uplink should be a module, but wlil think about this
 * futher
 */
void  uplink_read_callback(Socket * s)
{
    char	*line = NULL;
	log_message(LOG_DEBUG3, "Calling uplink read.");


	while (1)
	{
		log_message(LOG_DEBUG3, "Calling uplink read.");
		if (!(line = socket_getline(s)))
			break;
			
		parser_handle_line(line);
		free(line);
		line = NULL;
	}
}


/*********************************************************/


/*********************************************************/

