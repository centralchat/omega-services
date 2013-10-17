#ifndef __TOOLS_H__
#define __TOOLS_H__

unsigned int hash_safe(char *, ub4);
uint32 hash(char *);

#define HASH(x,y) hash_safe(x,y)


int generictoken(char, int, char *, char **);

#endif 