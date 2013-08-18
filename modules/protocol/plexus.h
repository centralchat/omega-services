#ifndef __plexus_H__
#define __plexus_H__

void PtclFactory();

void plexus_wallop  (char*,char*);
void plexus_connect ();
void plexus_ping    (char *);
void plexus_nick    (User*);
void plexus_version (User*);
void plexus_join    (User *, Channel *);
void plexus_part    (User *, Channel *, char *);
void plexus_xline   (char *, char *, char *, int, char *);
void plexus_numeric	(User *, int, char *);

static void plexus_mode (Link *, User *, Channel *, char *, char *);

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

#endif //__plexus_H_
