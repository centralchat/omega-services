#include "stdinc.h"
#include "server.h"

extern  void		sendq_add	(Socket *, char *, int, int);
extern  int	    	sendq_flush 	(Socket *);


void sendto_one (User *source, User *target, char *fmt, ...)
{
	va_list args;
	char    tmp[512];

	if (!target || !source)
		return;

	va_start (args, fmt);
	vsnprintf (tmp, 512, fmt, args);
	va_end (args);

	if (!tmp)
		return;

	source->lastsent	= time(NULL);

	send_line (":%s %s %s :%s", (IRCd.ts6) ? source->uid : source->nick, (CfgSettings.privmsg) ? "PRIVMSG" : "NOTICE",
                HasId(target) ? target->uid : target->nick, tmp);
	return;
}


void sendto_one_privmsg (User *source, User *target, char *fmt, ...)
{
    va_list args;
    char    tmp[512];

    if (!target || !source)
        return;

    va_start (args, fmt);
    vsnprintf (tmp, 512, fmt, args);
    va_end (args);

    if (!tmp)
        return;

    source->lastsent    = time(NULL);

    send_line (":%s PRIVMSG %s :%s", (IRCd.ts6) ? source->uid : source->nick,
                HasId(target) ? target->uid : target->nick, tmp);
    return;
}



void sendto_one_notice (User *source, User *target, char *fmt, ...)
{
    va_list args;
    char    tmp[512];

    if (!target || !source)
        return;

    va_start (args, fmt);
    vsnprintf (tmp, 512, fmt, args);
    va_end (args);

    if (!tmp)
        return;

    source->lastsent    = time(NULL);

    send_line (":%s NOTICE %s :%s", (IRCd.ts6) ? source->uid : source->nick,
                HasId(target) ? target->uid : target->nick, tmp);
    return;
}


void sendto_ircops (User *source, char *fmt, ...)
{
	va_list		args;
	char		tmp[512];
	dlink_node	*dl;
	User			*u;

	if (!source)
		return;

	va_start	(args, fmt);
	vsnprintf	(tmp, 512, fmt, args);
	va_end	(args);

	if (!tmp)
		return;

	DLINK_FOREACH (dl, userlist.head)
	{
		u	= (User *) dl->data;

		if (u->oper)
			sendto_one_notice (source, u, "%s", tmp);
	}
	return;
}




void sendto_one_help(User *src, User *trg, char *cmd, char *fmt,...)
{
	va_list args;
	char    tmp[512], padding[13];
	int s_cmd = 0;
	char *ptr;
	
	if (!trg || !src)
		return;

 	if (sync_state == BURSTING)
 	       return;
 	       
	memset(padding, '\0', sizeof(padding));
	
	va_start (args, fmt);
	vsnprintf (tmp, 512, fmt, args);
	va_end (args);
	
	ptr = padding;
	for (s_cmd = strlen(cmd); s_cmd < sizeof(padding); s_cmd++) { 
		*ptr = ' ';
		ptr++;
	}
	sendto_one(src, trg, "  %s%s - %s", cmd, padding, tmp); 
	return;
}


void sendto_channel(User* src,Channel* trg, char* fmt,...)
{
	va_list args;
	char    tmp[512];

	if (!trg || !src)
		return;

 	if (sync_state == BURSTING)
 	       return;

	va_start (args, fmt);
	vsnprintf (tmp, 512, fmt, args);
	va_end (args);

	src->lastsent	= time(NULL);

	send_line (":%s PRIVMSG %s :%s", (IRCd.ts6) ? src->uid : src->nick, trg->name, tmp);

	return;
}

void sendto_one_numeric (char *source, User *target, int numeric, char *fmt, ...)
{
    va_list     args;
    char        tmp[512];

    if (!source || !target)
        return;

    va_start    (args, fmt);
    vsnprintf   (tmp, 512, fmt, args);
    va_end      (args);
	
    ircd_numeric(target, numeric, tmp);
	
#if 0
    if ((strcasecmp (CfgSettings.protocol, "inspircd12")) == 0)
        send_line (":%s PUSH %s ::%s %03d %s %s", Omega->me->sid, target->uid, Omega->me->nam be, numeric, target->uid, tmp);
    else
        send_line (":%s %d %s %s", (IRCd.ts6) ? Omega->me->sid : Omega->me->name, numeric, HasId(target) ? target->uid : target->nick, tmp);
#endif
 
    return;
}



void sendto_server (User *source, char *fmt, ...)
{
	va_list		args;
	char		tmp[512];

	va_start	(args, fmt);
	vsnprintf	(tmp, 512, fmt, args);
	va_end		(args);

	send_line (":%s %s", (source)? HasId(source)? source->uid : source->nick : CfgSettings.servername, args);

	return;
}



int sendto_socket (Socket *sock, char *fmt, ...)
{
	char	tmp[1024];
	int		n;
	va_list	args;
	dlink_node	*dl;

	if ((!sock) || (sock->sd <= 0))
		return 1;

	memset (tmp, '\0', sizeof(tmp));

	va_start (args, fmt);
	vsnprintf (tmp, 1024, fmt, args);
	va_end (args);

    alog (LOG_DEBUG2, "IO/OUT: %s: %s", sock->name, tmp);

#if 0
    if (strstr (tmp, "Relay -"))
        sendto_channel(find_user("Relay"), find_channel("#Relay"), "%s: %s", sock->name, tmp);
#endif

	if (!strstr (tmp, "\n"))
		strlcat (tmp, "\n", sizeof(tmp));

        /* This should prevent sending from blocking */
    n       = sendq_flush (sock);

    if (n == -1)
    {
            if (errno != EAGAIN)
            {
                alog (LOG_ERROR, "write error on %s: %s", sock->name, strerror(errno));
                //if (sock->error_callback)
                //    sock->error_callback (sock);
				close (sock->sd);
                sock->sd    = -1;
                sock->flags |= SOCK_DEAD;
                /*
				dl = dlink_find_delete (sock, &sockets);
				dlink_free (dl);
				free (sock);
                */
                return 1;
            }
            else
            {
                sendq_add (sock, tmp, strlen(tmp), 0);
                return 0;
            }
    }
    else if (n == 0)
    {
        sendq_add (sock, tmp, strlen(tmp), 0);
        return 0;
    }

    if ((n = write (sock->sd, tmp, strlen(tmp))) == -1)
    {
        if (errno != EAGAIN)
        {
            alog (LOG_ERROR, "write error on %s: %s", sock->name, strerror(errno));
            /*
            if (sock->error_callback)
                sock->error_callback (sock);
            */
			close (sock->sd);

            sock->sd    = -1;
            sock->flags |= SOCK_DEAD;
            /*
			dl = dlink_find_delete (sock, &sockets);
			dlink_free (dl);
			free (sock);
            */
            return 1;
        }
        else
        {
			sendq_add (sock, tmp, strlen(tmp), 0);
            return 0;
        }
    }
    sentbytes += n;

    if (n != strlen(tmp))
        sendq_add (sock, tmp, strlen(tmp), n);

    return 0;
}
