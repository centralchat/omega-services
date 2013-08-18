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
 *    $Id: protocol.c 2182 2012-01-05 02:13:53Z  $
 */


#include "stdinc.h"
#include "server.h"
#include "extern.h"


#define NAME "m_stats"
#define AUTHOR "Jer"
#define VERSION "$Id: m_stats.c 1942 2010-06-25 07:25:07Z twitch $"


extern IRCDProto IRCd;

void m_stats (User *, int , char **);

int stats_load();
void stats_close();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),stats_load, stats_close);

int stats_load()
{
      AddUserCmd ("STATS", m_stats);
      return MOD_ERR_OK;
}


void stats_close()
{
	DelUserCmd("STATS", m_stats);
	return;
}

void m_stats (User *user, int argc, char **argv)
{
	long total;
	Access* ca;
	dlink_node *dl,*hdl;
	char* host;


    if (!check_access(user, ACC_FLAG_LIST))
    {
		sendto_one_numeric(CfgSettings.servername, user, 481, ":Permission Denied - You do not have permissions to continue");
		return;

    }	
    switch (*argv[1])
    {
	case 'O':
	case 'o':
		DLINK_FOREACH(dl,accesslist.head)
		{
			 ca = dl->data;

			DLINK_FOREACH(hdl,ca->hosts.head)
			{
				host = hdl->data;
				 sendto_one_numeric(CfgSettings.servername,user, 243,":O %s %s %s",ca->name,
						(check_access(user, ACC_FLAG_LIST))? host : "*",ca->flags);

			}

		}

		break;
	 case 'S':
		    if (!check_access(user, ACC_FLAG_LIST))
		    {
			    sendto_one_numeric(CfgSettings.servername,user,481,":Permission Denied. You do not have the correct permissions.");
			    break;
		    }

		    sendto_one_numeric(CfgSettings.servername, user, 304, ":Database::Name %s", (db)? db->name : "none"); 
		    sendto_one_numeric(CfgSettings.servername,user,304,":Protocol::Name %s",IRCd.name);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Protocol::TS6 %s",(IRCd.ts6)? "Yes" : "No");
		     
                    sendto_one_numeric(CfgSettings.servername,user,304,":Server::Servername %s",CfgSettings.servername);
    		    sendto_one_numeric(CfgSettings.servername,user,304,":Server::Sid %s",(IRCd.ts6)? CfgSettings.sid : "Non TS6 Support");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Server::Numeric %s",CfgSettings.numeric);

		    sendto_one_numeric(CfgSettings.servername,user,304,":Link::Uplink %s",CfgSettings.uplink);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Link::Port %d",CfgSettings.port);
		   
		    sendto_one_numeric(CfgSettings.servername,user,304,":Logging::Channel %s",logchan);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Logging::Connects %s",(logclients)? "Yes" : "No");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Logging::Nicks %s",(lognicks)? "Yes" : "No");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Logging::Channels %s",(logchannels)? "Yes" : "No");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Logging::Commands %s",(logcmds)? "Yes" : "No");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Settings::OperOnly %s",(CfgSettings.operonly)? "Yes" : "No");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Settings::LocalIP %s",CfgSettings.local_ip);			
		    sendto_one_numeric(CfgSettings.servername,user,304,":Settings::Message %s",(CfgSettings.privmsg)? "PRIVMSG" : "NOTICE");
		    sendto_one_numeric(CfgSettings.servername,user,304,":Settings::GNUTLS %s", (CfgSettings.gnutls)? "Enabled" : "Disabled");


		    sendto_one_numeric(CfgSettings.servername,user,304,":Guardian::Nick %s",guardian.nick);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Guardian::User %s",guardian.user);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Guardian::Host %s",guardian.host);
		    sendto_one_numeric(CfgSettings.servername,user,304,":Guardian::Description %s",guardian.desc);
		    break;
        case 'u':
            sendto_one_numeric (CfgSettings.servername, user, 242, ":Server Up: %s", timediff(ServerTime - starttime));
            break;
        case 'T':
        case 't':
            sendto_one_numeric (CfgSettings.servername, user, 249, ":Bytes in: %d, Bytes out: %d", recvbytes, sentbytes);
	        sendto_one_numeric (CfgSettings.servername, user, 249, ":User count: %d, size: %d", memcounts.users, memcounts.userssize);
			sendto_one_numeric (CfgSettings.servername, user, 249, ":dlink count: %d, size: %d", dlink_count, dlink_alloc);
			sendto_one_numeric (CfgSettings.servername, user, 249, ":Channel count: %d, size: %d", memcounts.channels, memcounts.chansize);
			sendto_one_numeric (CfgSettings.servername, user, 249, ":Channel User count: %d, size: %d", memcounts.chanucnt, memcounts.chanusize);
			sendto_one_numeric (CfgSettings.servername, user, 249, ":Server count: %d, size: %d", memcounts.servcnt, memcounts.servsize);
            sendto_one_numeric (CfgSettings.servername, user, 249, ":Events count: %d, size: %d", memcounts.eventscnt, memcounts.eventssize);
            sendto_one_numeric (CfgSettings.servername, user, 249, ":Message Buffer count: %d, size: %d", memcounts.msgbufcnt, memcounts.msgbufsize);
            sendto_one_numeric (CfgSettings.servername, user, 249, ":Buffered Lines count: %d, size: %d", memcounts.buflinecnt, memcounts.buflinesize);
	        sendto_one_numeric (CfgSettings.servername, user, 249, ":Session List Count: %d, size: %d",session_count, (int) session_size);
			total	= memcounts.userssize + dlink_alloc + memcounts.chansize + memcounts.chanusize + memcounts.servsize + memcounts.msgbufsize + memcounts.buflinesize;

#ifdef HAVE_PTHREAD
		    sendto_one_numeric (CfgSettings.servername, user, 249, ":Threaded Objects: %d", thread_count);
#endif

			sendto_one_numeric (CfgSettings.servername, user, 249, ":Total: %d", total);
            break;
    }

    sendto_one_numeric (CfgSettings.servername, user, 219, "%c :End of /STATS report", *argv[1]);

    sendto_logchan ("Stats '%c' requested by %s", *argv[1], user->nick);
    return;
}

/* vim: set tabstop=4 shiftwidth=4 expandtab */

