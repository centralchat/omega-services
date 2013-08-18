/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	        (C) 2008-2012 Omega Dev Team
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
 *    $Id: config.c 2325 2012-01-31 18:41:47Z jer $
 */

/*
 * Dynamic Configuration Notes:
 *
 * We have one major problem to solve regarding dynamic configurations,
 * we have to solve the chicken and the egg problem, To know what modules
 * we need to load we must first read the config, if these modules happen
 * to insert config options, and require them to initialize we run into
 * the problem of, how do we account for these config values AFTER the
 * config has been loaded, reloading the config for each module load is
 * just a plain bad idea. I have a few theories but this definitely needs
 * alot of for thought.
 *
 */

#include "stdinc.h"
#include "configparse.h"
#include "server.h"



#define PARAM_ERROR(ce, s)  { fprintf (stderr, "%s:%i: no parameter for %s " \
                                    "config option: %s\n", (ce)->ce_fileptr->cf_filename, \
                                    (ce)->ce_varlinenum, (s), (ce)->ce_varname); }


#define INVALID_PARAM(ce, s)  { fprintf (stderr, "%s:%i: Invalid parameter %s " \
                                    "config option: %s\n", (ce)->ce_fileptr->cf_filename, \
                                    (ce)->ce_varlinenum, (s), (ce)->ce_varname); \ }

#define FOREACH_PARAM(ce) for (ce = ce->ce_entries; ce; ce = ce->ce_next)

int modcnt = 0;

extern char p10_id[3];

	/**
		<server name="security.network.tld" sid="44x" numeric="9">
		<link name="dev.ircnetwork.tld" port="7020" pass="l|nkp@55">
	**/

void InitDefaults ()
{
	strcpy (CfgSettings.servername, "security.network.tld");
	strcpy (CfgSettings.sid, "666");
	strcpy (CfgSettings.numeric, "6");
	strcpy (CfgSettings.uplink, "hub.network.tld");
	strcpy (CfgSettings.pass, "password");
	strcpy (CfgSettings.network, "Misconfigured");

	strlcpy (CfgSettings.umode, "+o", sizeof(CfgSettings.umode));


	CfgSettings.port = 6667;
	strcpy (CfgSettings.desc, "Security Services");
   	

	logclients = 0;
	logcmds	= 0;

    	CfgSettings.privmsg	= 0;
    	CfgSettings.operonly = 1;

	snprintf (datadir, sizeof(datadir), "%s", DBDIR);
}

void VerifyConf ()
{
    char *ce;
    struct stat     buf;
    int             retval;

	
	//first off apply our information before we run our checks on it.
	if ((ce = (char *)get_config_entry("server", "name")))
		strlcpy (CfgSettings.servername, ce,sizeof(CfgSettings.servername));
	if ((ce = (char *)get_config_entry("server", "sid")))
		strlcpy (CfgSettings.sid, ce, sizeof(CfgSettings.sid));
	if ((ce = (char *)get_config_entry("server", "numeric")))
		strlcpy (CfgSettings.numeric, ce, sizeof(CfgSettings.numeric));
	if ((ce = (char *)get_config_entry("link", "name")))
		strlcpy (CfgSettings.uplink, ce, sizeof(CfgSettings.uplink));
	if ((ce = (char *)get_config_entry("link", "pass")))
		strlcpy (CfgSettings.pass, ce,sizeof(CfgSettings.pass));	
	if ((ce = (char *)get_config_entry("link", "network")))
		strlcpy (CfgSettings.network, ce, sizeof(CfgSettings.network));	
	if ((ce = (char *)get_config_entry("link", "port")))
			CfgSettings.port = atoi(ce);	 		
	if ((ce = (char *)get_config_entry("settings", "ip")))
		strlcpy(CfgSettings.local_ip,ce,sizeof(CfgSettings.local_ip));
	if ((ce = (char *) get_config_entry("settings","pidfile")))
		snprintf (CfgSettings.pidfile, sizeof(CfgSettings.pidfile), "%s/%s", DPATH, ce);
	if ((ce = (char *) get_config_entry("logging", "channel")))
		strlcpy(logchan,ce,sizeof(logchan));
	if ((ce = (char *) get_config_entry("settings", "umode")))
		strlcpy (CfgSettings.umode, ce, sizeof(CfgSettings.umode));
         
	 logcmds = 0; lognicks = 0; logclients = 0; logchannels = 0;
	
	 CfgSettings.operonly = get_config_bool("settings", "operonly", 1);
	 CfgSettings.privmsg = get_config_bool("settings", "privmsg", 0);
	 CfgSettings.gnutls = get_config_bool("settings", "gnutls", 0);

	if (CfgSettings.local_host != NULL)
		free (CfgSettings.local_host);

	if (CfgSettings.local_ip[0] == '\0')
		CfgSettings.local_host = get_hostname (NULL);
	else if (match_regx (CfgSettings.local_ip, "^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$"))
		CfgSettings.local_host = get_hostname (NULL);
	else
		CfgSettings.local_host = get_hostname (CfgSettings.local_ip);
    

    if (CfgSettings.numeric != NULL)
        inttobase64 (p10_id, atoi(CfgSettings.numeric), 2); // This should be safe, numeric should be denied in rehashes

    if ((strcasecmp (CfgSettings.network, "Misconfigured")) != 0)
    {
        snprintf (datadir, sizeof(datadir), "%s/%s", DBDIR, CfgSettings.network);

        retval  = stat (datadir, &buf);

        if (retval != 0)
        {
            switch (retval)
            {
                case EACCES:
                case ELOOP:
                case ENAMETOOLONG:
                case ENOMEM:
                    fprintf (stderr, "Warning: datadir %s does not exist.\n", datadir);
                    break;
                case ENOTDIR:
                    if (mkdir (datadir, 0755) != 0)
                        fprintf (stderr, "Unable to create datadir %s: %s\n", datadir, strerror(errno));
                    break;

                default:
                    fprintf (stderr, "stat() error: %s\n", strerror(errno));

                    if (mkdir (datadir, 0755) != 0)
                        fprintf (stderr, "Unable to create datadir %s: %s\n", datadir, strerror(errno));
            }
        }
    }
    else
    {
        fprintf (stderr, "Warning: network Misconfigured using %s as datadir\n", datadir);
        // Warn about using DBDIR to store databases
    }
}

void init_config() {
/*
    dlink_node *dl;

    struct ConfTable *ct = NULL;

    for (ct = conf_root_table; ct->name; ct++)
            add_config_table(ct, CONF_REQUIRED);
*/
	return;
}
