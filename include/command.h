#ifndef __COMAND_H__
#define __COMAND_H__

#define COMMAND_PRIME 2047
#define COMMAND_HASH(str) hash_safe((str), COMMAND_PRIME)

#define COMMAND_TYPE_SERVER 0
#define COMMAND_TYPE_CLIENT 1

typedef struct {
	char *event_string;
	int (*handler)(void*,args_t *);
} command_t;

dlink_list commands[2][COMMAND_PRIME];




#endif //__COMAND_H__