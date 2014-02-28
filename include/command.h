#ifndef __COMAND_H__
#define __COMAND_H__

#define COMMAND_PRIME 2047
#define COMMAND_HASH(str) hash_safe((str), COMMAND_PRIME)

#define COMMAND_LIST_SRV srvcommands
#define COMMAND_LIST_USR usrcommands
#define COMMAND_LIST_CLI clicommands


typedef struct {
	char * event_string;
	int (*handler)(args_t *);
} command_t;

dlink_list usrcommands[COMMAND_PRIME];
dlink_list srvcommands[COMMAND_PRIME];
dlink_list clicommands[COMMAND_PRIME];

#define   command_server_add(str, handler) command_add  (COMMAND_LIST_SRV, str, handler)
#define   command_server_del(str, handler) command_del  (COMMAND_LIST_SRV, str, handler)
#define   command_server_emit(str, args)   command_emit (COMMAND_LIST_SRV, str, args)

#define   command_user_add(str, handler)  command_add  (COMMAND_LIST_USR, str, handler)
#define   command_user_del(str, handler)  command_del  (COMMAND_LIST_USR, str, handler)
#define   command_user_emit(str, args)    command_emit (COMMAND_LIST_USR, str, args)

#define   command_cli_add  (str, handler) command_add  (COMMAND_LIST_CLI, str, handler)
#define   command_cli_del  (str, handler) command_del  (COMMAND_LIST_CLI, str, handler)
#define   command_cli_emit (str, args)    command_emit (COMMAND_LIST_CLI, str, args)

int         command_add  (dlink_list*, char *, int (*)(args_t *));
void        command_del  (dlink_list*, char *, int (*)(args_t *));
int         command_emit (dlink_list*, char *, args_t *);
command_t * command_new  (char *, int (*)(args_t *));
void        command_free (command_t *);




#endif //__COMAND_H__