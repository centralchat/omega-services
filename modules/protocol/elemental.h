#ifndef __elemental_H__
#define __elemental_H__

void PtclFactory();

void elemental_wallop  (char*,char*);
void elemental_connect ();
void elemental_ping    (char *);
void elemental_nick    (User*);
void elemental_version (User*);
void elemental_join    (User *, Channel *);
void elemental_part    (User *, char *, char *);
void elemental_xline   (char *, char *, char *, int, char *);
void elemental_numeric(User *, int, char *);

static void elemental_mode (Link *, User *, Channel *, char *, char *);

static void omega_nick      (Link *, int, char **);
static void omega_unick	    (User *, int, char **);
static void omega_uid       (Link *, int, char **);
static void omega_quit      (User *, int, char **);
static void omega_sjoin     (Link *, int, char **);
static void omega_privmsg   (User *, int, char **);
static void omega_server    (Link *, int, char **);
static void omega_mode      (Link *, int, char **);
static void omega_ping      (Link *, int, char **);
static void omega_pong      (Link *, int, char **);
static void omega_sid       (Link *, int, char **);
static void omega_pass      (Link *, int, char **);
static void omega_squit     (Link *, int, char **);
static void omega_encap     (Link *, int, char **);
static void omega_mode_user (User *, int, char **);
static void omega_tmode     (User *, int, char **);
static void omega_part      (User *, int, char **);
static void omega_join      (User *, int, char **);
static void omega_kill	    (User *, int, char **);
static void omega_skill	    (Link *, int, char **);
static void omega_rehash    (User *, int, char **);
static void omega_null	    (void *, int, char **);

static void omega_forcejoin (User *, int, char **);
static void omega_forcepart (User *, int, char **);
static void omega_invite    (User *, int, char **);

#endif //__elemental_H_
