#ifndef __RATBOX_H__
#define __RATBOX_H__

void PtclFactory();

void kcircd_wallop  (char*,char*);
void kcircd_connect ();
void kcircd_ping    (char *);
void kcircd_nick    (User*);
void kcircd_version (User*);
void kcircd_join    (User *, Channel *);
void kcircd_numeric	(User *, int, char *);

static void kcircd_mode (Link *, User *, Channel *, char *, char *);

static void omega_null		(Link *, int, char **);

static void omega_nick      (Link *, int, char **);
static void omega_unick		(User *, int, char **);
static void omega_uid       (Link *, int, char **);
static void omega_quit      (User *, int, char **);
static void omega_sjoin     (Link *, int, char **);
static void omega_privmsg   (User *, int, char **);
static void omega_server    (Link *, int, char **);
static void omega_mode      (Link *, int, char **);
static void omega_ping      (Link *, int, char **);
static void omega_sid       (Link *, int, char **);
static void omega_pass      (Link *, int, char **);
static void omega_squit     (Link *, int, char **);
static void omega_encap     (Link *, int, char **);
static void omega_mode_user (User *, int, char **);
static void omega_tmode     (User *, int, char **);
static void omega_part		(User *, int, char **);
static void omega_join		(User *, int, char **);
static void omega_kill		(User *, int, char **);
static void omega_skill		(Link *, int, char **);

#endif //__RATBOX_H_
