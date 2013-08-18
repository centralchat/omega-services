#ifndef __RATBOX_H__
#define __RATBOX_H__

void PtclFactory();

void ratbox_wallop  (char*,char*);
void ratbox_connect ();
void ratbox_ping    (char *);
void ratbox_nick    (User*);
void ratbox_version (User*);
void ratbox_join    (User *, Channel *);
void ratbox_part    (User *, Channel *, char *);
void ratbox_xline   (char *, char *, char *, int, char *);
void ratbox_server  (Link *);
void ratbox_numeric (User*, int, char *);
void ratbox_kill    (User *, char *);

static void ratbox_mode (Link *, User *, Channel *, char *, char *);

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

#endif //__RATBOX_H_
