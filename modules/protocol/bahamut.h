#ifndef __BAHAMUT_H__
#define __BAHAMUT_H__

void bahamut_connect	();
void bahamut_ping		(char *);
void bahamut_nick		(User *);
void bahamut_join		(User *, Channel *);

static void omega_null		(void *, int, char **);
static void omega_server	(Link *, int, char **);
static void omega_svinfo	(Link *, int, char **);
static void omega_ping		(Link *, int, char **);
static void omega_nick		(void *, int, char **);
static void omega_sjoin		(Link *, int, char **);

#endif /* __BAHAMUT_H__ */
