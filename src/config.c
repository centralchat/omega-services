#include "stdinclude.h"
#include "appinclude.h"


void load_config_defaults() { 
	sprintf (core.settings.pid_file,   "%s/security.pid", VAR_DIR);
	sprintf (core.settings.config_file,"%s/security.cfg", CONFIG_DIR);

	strcpy (core.settings.local_ip,  "127.0.0.1");

	core.settings.port = 2999;
}


void load_config_values(void) { 
	char * ce; 
	int ce_int = 0;

	if ((ce = (char *)get_config_entry("settings", "pidfile")))
		snprintf (core.settings.pid_file, sizeof(core.settings.pid_file), "%s/%s", VAR_DIR, ce);
	else
		snprintf (core.settings.pid_file, sizeof(core.settings.pid_file), "%s/app.pid", VAR_DIR);

    if ((ce = (char *)get_config_entry("bind", "host")))
		strlcpy (core.settings.local_ip,  ce,  sizeof(core.settings.local_ip));		


	if ((ce = (char *)get_config_entry("link", "name")))
		strlcpy (core.settings.con_host,  ce,  sizeof(core.settings.local_ip));		

	core.settings.con_port = get_config_int("link", "port", 7080);
	core.settings.port     = get_config_int("bind", "port", 2990);
	max_sockets            = get_config_int("settings", "maxfds", 1024);

	// ConfBase *cb;

	// dlink_list *config_info;
	// dlink_node *dl, *tdl;

	// db_info_t * dbi;

	// config_info = get_config_base("database");
	// DLINK_FOREACH(dl, config_info->head) { 
	// 	char * host   = NULL;
	// 	char * file   = NULL;
	// 	char * pass   = NULL; 
	// 	char * user   = NULL;
	// 	char * driver = NULL;

	// 	cb = tdl->data;

	// 	host   = get_config_entry_by_cb(cb, "host");
	// 	driver = get_config_entry_by_cb(cb, "host");
	// 	if (!host) {
	// 		file = get_config_entry_by_cb(cb, "file");
	// 	} else { 
	// 		pass = get_config_entry_by_cb(cb, "host");
	// 		user = get_config_entry_by_cb(cb, "host");
	// 	}
		
	// 	if (!(dbi = malloc(sizeof(db_info_t))))
	// 		break;

	// 	strlcpy(dbi->host,sizeof(dbi->host), host ? host : "localhost");
	// 	strlcpy(dbi->pass,sizeof(dbi->pass), pass ? pass : "root");
	// 	strlcpy(dbi->user,sizeof(dbi->pass), user ? user : "root");
	// 	strlcpy(dbi->file,sizeof(dbi->file), file ? file : " ");
	// 	strlcpy(dbi->driver,sizeof(dbi->driver), driver ? driver : " ");


	// 	tdl = dlink_create();
	// 	dlink_add_tail(dbi, tdl, &cfg_settings.databases);
		
	// }

}
