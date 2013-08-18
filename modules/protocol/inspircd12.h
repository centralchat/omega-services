#ifndef __INSPIRCD12_H__
#define __INSPIRCD12_H__

int Module_Init ();

static int inspircd12_init  ();

void inspircd12_connect ();
void inspircd12_ping    (char *);
void inspircd12_nick    (User *);
void inspircd12_fjoin   (User *, Channel *);
void inspircd12_part    (User *, Channel *, char *);
void inspircd12_ping    (char *);
void inspircd12_wallop  (char *, char *);
void inspircd12_server  (Link *);
void inspircd12_xline   (char *, char *, char *, int, char *);
static void inspircd12_numeric (User *, int, char *);
void inspircd12_kill	(User *, char *);

static void omega_null      (void *, int, char **);

static void omega_ping      (Link *, int, char **);
static void omega_pong      (Link *, int, char **);
static void omega_capab     (Link *, int, char **);
static void omega_server    (Link *, int, char **);
static void omega_burst     (Link *, int, char **);
static void omega_endburst  (Link *, int, char **);
static void omega_uid       (Link *, int, char **);
static void omega_fjoin     (Link *, int, char **);
static void omega_squit     (Link *, int, char **);
static void omega_svsnick   (Link *, int, char **);
static void omega_version   (Link *, int, char **);
static void omega_error     (Link *, int, char **);

static void omega_privmsg   (User *, int, char **);
static void omega_part      (User *, int, char **);
static void omega_invite    (User *, int, char **);
static void omega_quit      (User *, int, char **);
static void omega_fmode     (User *, int, char **);
static void omega_kill      (User *, int, char **);
static void omega_encap     (User *, int, char **);
static void omega_mode      (User *, int, char **);
static void omega_opertype  (User *, int, char **);
static void omega_kick      (User *, int, char **);
static void omega_fhost     (User *, int, char **);
static void omega_fname     (User *, int, char **);
static void omega_fident    (User *, int, char **);
static void omega_nick      (User *, int, char **);

#endif //__INSPIRCD12_H__
