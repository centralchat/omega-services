/*
 *         	  OMEGA IRC SECURITY SERVICES
 * 	      	    (C) 2008-2012 Omega Dev Team
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *    $Id: g_scan.c 2394 2012-06-30 15:56:22Z twitch $
 */

#include "stdinc.h"
#include "server.h"

#define NAME "m_scan"
#define VERSION "Omega threat detection module v0.1"
#define AUTHOR "Twitch"


int scan_debug = 1;

struct scanque_ {
	User* user;
	struct scanque_ *next,*prev;
};

char *scan_ignore[512];
int scan_ignore_cnt = 0;

dlink_list ScanQ;
dlink_list VersionQ;

#if 0
static void scan_nick(Link*,int,char**);
#endif
static int scan_nick (int, void *);

int scan_que(int, void*);
int version_que(int, void*);
int init_events(int, void*);

int HandleResult(User* u, int score);
void write_bl_log(User* u, int score);

static void cmd_version   (User *, int, char **);
static void cmd_quit      (User *, int, char **);
static void cmd_scan      (User *, int, char **);

int has_q = 0; //do we have a scanq to parse?

FILE *fp;

int Module_Init();
void Module_Close();

MODHEADER(NAME, VERSION, AUTHOR, MAKE_ABI(0,6,4),Module_Init, Module_Close);


float avg_range = 15.525;
float nick_range_val = 0;
int nick_count = 0;
int scan_action = 0;


void help_scan(User*);
void help_scan_ext(User*);

//map for old style checking
//this contains our point value for various
//character

int OldCharMap[128] = {
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused, Unused, Horizontal Tab */
    0, 0, 0,                    /* Line Feed, Unused, Unused */
    0, 0, 0,                    /* Carriage Return, Unused, Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused, Unused, Space */
    0, 0, 0,                    /* ! " #  */
    0, 0, 0,                    /* $ % &  */
    0, 0, 0,                    /* ! ( )  */
    0, 0, 0,                    /* * + ,  */
    0, 0, 0,                    /* - . /  */
    1, 1,                       /* 0 1 */
    1, 1,                       /* 2 3 */
    1, 1,                       /* 4 5 */
    1, 1,                       /* 6 7 */
    1, 1,                       /* 8 9 */
    0, 0,                       /* : ; */
    0, 0, 0,                    /* < = > */
    0, 0,                       /* ? @ */
    -1, -1, -1,                 /* A B C */
    -1, -1, -1,                 /* D E F */
    -1, -1, -1,                 /* G H I */
    -1, -1, -1,                 /* J K L */
    -1, -1, -1,                 /* M N O */
    -1, -1, -1,                 /* P Q R */
    -1, -1, -1,                 /* S T U */
    -1, -1, -1,                 /* V W X */
    -1, -1,                     /* X Z   */
    0, 0, 0,                    /* [ \ ] */
    0, 0, 0,                    /* ^ _ ` */
    -1, -1, -1,                 /* a b c */
    -1, -1, -1,                 /* d e f */
    -1, -1, -1,                 /* g h i */
    -1, -1, -1,                 /* j k l */
    -1, -1, -1,                 /* m n o */
    -1, -1, -1,                 /* p q r */
    -1, -1, -1,                 /* s t u */
    -1, -1, -1,                 /* v w x */
    -1, -1,                     /* y z */
    0, 0, 0,                    /* { | } */
    0, 0                        /* ~ ‚ */
};



int CharMap[128] = {
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused, Unused, Horizontal Tab */
    0, 0, 0,                    /* Line Feed, Unused, Unused */
    0, 0, 0,                    /* Carriage Return, Unused, Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                /* Unused, Unused, Space */
    0, 0, 0,                    /* ! " #  */
    0, 0, 0,                    /* $ % &  */
    0, 0, 0,                    /* ! ( )  */
    0, 0, 0,                    /* * + ,  */
    0, 0, 0,                    /* - . /  */
    0, 0,                       /* 0 1 */
    0, 0,                       /* 2 3 */
    0, 0,                       /* 4 5 */
    0, 0,                       /* 6 7 */
    0, 0,                       /* 8 9 */
    0, 0,                       /* : ; */
    0, 0, 0,                    /* < = > */
    0, 0,                       /* ? @ */
    1, 0, 1,                    /* A B C */
    2, 1, 1,                    /* D E F */
    2, 2, 1,                    /* G H I */
    2, 2, 1,                    /* J K L */
    1, 1, 1,                    /* M N O */
    2, 2, 1,                    /* P Q R */
    1, 1, 1,                    /* S T U */
    3, 2, 3,                    /* V W X */
    3, 3,                       /* X Z   */
    0, 0, 0,                    /* [ \ ] */
    0, 0, 0,                    /* ^ _ ` */
    1, 2, 1,                    /* a b c */
    0, 1, 1,                    /* d e f */
    2, 2, 1,                    /* g h i */
    3, 2, 1,                    /* j k l */
    1, 1, 1,                    /* m n o */
    2, 3, 1,                    /* p q r */
    1, 1, 1,                    /* s t u */
    3, 2, 3,                    /* v w x */
    3, 3,                       /* y z */
    0, 0, 0,                    /* { | } */
    0, 0                        /* ~ ‚ */
};

int SpecialCharMap[128] = {
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused, Unused, Horizontal Tab */
    0, 0, 0,                    /* Line Feed, Unused, Unused */
    0, 0, 0,                    /* Carriage Return, Unused, Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused */
    0, 0, 0,                    /* Unused, Unused, Space */
    1, 1, 1,                    /* ! " #  */
    1, 1, 1,                    /* $ % &  */
    1, 1, 1,                    /* ! ( )  */
    1, 1, 1,                    /* * + ,  */
    1, 1, 1,                    /* - . /  */
    0, 0,                       /* 0 1 */
    0, 0,                       /* 2 3 */
    0, 0,                       /* 4 5 */
    0, 0,                       /* 6 7 */
    0, 0,                       /* 8 9 */
    1, 1,                       /* : ; */
    1, 1, 1,                    /* < = > */
    1, 1,                       /* ? @ */
    0, 0, 0,                    /* A B C */
    0, 0, 0,                    /* D E F */
    0, 0, 0,                    /* G H I */
    0, 0, 0,                    /* J K L */
    0, 0, 0,                    /* M N O */
    0, 0, 0,                    /* P Q R */
    0, 0, 0,                    /* S T U */
    0, 0, 0,                    /* V W X */
    0, 0,                       /* X Z   */
    1, 1, 1,                    /* [ \ ] */
    1, 1, 1,                    /* ^ _ ` */
    0, 0, 1,                    /* a b c */
    0, 0, 1,                    /* d e f */
    0, 0, 1,                    /* g h i */
    0, 0, 1,                    /* j k l */
    0, 0, 1,                    /* m n o */
    0, 0, 1,                    /* p q r */
    0, 0, 1,                    /* s t u */
    0, 0, 0,                    /* v w x */
    0, 0,                       /* y z */
    1, 1, 1,                    /* { | } */
    1, 1                        /* ~ ‚ */
};


int Module_Init()
{

	int i;
	char *ce, *start;
	char tmp[512];
	char path[255];
	int status = 0;

	/*
	if (status > 0)
		 throwModuleErr("Unable to register module (%d)",status);
	*/


	AddEvent ("USERCONN", scan_nick);
	AddEvent ("BURST", init_events);

    AddUserCmd("NOTICE", cmd_version);

	AddCmd(s_Guardian, "SCAN", ACC_FLAG_OPER, cmd_scan, help_scan_ext, 1);
	AddHelp(s_Guardian, help_scan);
	

	sprintf(path,"%s%s%s",DPATH,LPATH,"scan.log");
	fp = fopen(path,"a+");
	scan_action = get_config_bool("scan", "kill", 0);

	memset(path,0,sizeof(path));
	return 0;
}


void Module_Close()
{

	int i = 0;
	for (i = 0; i < scan_ignore_cnt; i++)
		free(scan_ignore[i]);

	DelUserCmd("NOTICE", cmd_version);
	DelEvent ("USERCONN", scan_nick);

	//just in case we aren't deleted.
	DelTimedEvent("VersionQueue");
	DelTimedEvent ("SCANQ");

    DelCmd(s_Guardian, "SCAN", cmd_scan, help_scan_ext);
	DelHelp(s_Guardian, help_scan);
	return;
}

int init_events (int ac, void *nil)
{
	AddTimedEvent ("SCANQ", scan_que, 30);
	AddTimedEvent ("VersionQueue", version_que, 30);
	return EVENT_OK;
}


/************************************************/
/**
 * We need to be very VERY careful as to not
 * destroy the string or else we fail at life
 * so since we are called AFTER the protocol
 * handles it, use the user struct for the
 * given nick.
 */

static int scan_nick (int argc, void *user)
{
	User *u = user;
    char *tmp = NULL;
	int ig_cnt = 0;
	char string[512];

	dlink_node	*dl;
	struct scanque_	*sq;
	
	int in_section = 0; // 0 = Nick 1 = Ident 2 = Host 3 = Name


         int score  = 0;
	 int score2 = 0;

	 int special_char = 0;
	 int misc_checks  = 0;
	 int cmd_check    = 0;
	 

	

	 float total = 0.0;

         int len = 0;                   //might as well count the len since were looping.
	 float average = 0;
	 float alloted_prec = 0;
	 float percent = 0;

	if (MyConnect(u))
	{
		if (cmd_check == 0)
			return;
	}

	if (argc > 1)
	{
		sendto_logchan("Performing Scan on %s",u->nick);
		cmd_check = 1;
	}
	


	/**
	 ** Since I want to catch versions early on, send it
	 */

        if (sync_state == 3)
        	send_line(":%s PRIVMSG %s :\1VERSION\1",s_Guardian->nick,u->nick);
	else
	{
		sq	= (struct scanque_ *) malloc (sizeof(struct scanque_));
		dl	= dlink_create ();

		sq->user	= u;

		dlink_add_tail (sq, dl, &VersionQ);
	}
	if (u->service)
              return;

	snprintf(string, 511, "%s!%s@%s:%s", u->nick, u->user, (u->host)? u->host : u->ip, u->realname);
	
    /**
     * First off start scanning for old style attackers
     * 5 - 6 letter random alphanumeric nicks and
     * realnames, were the nick/realname have
     * more numbers then letters...
     * Note: We don't care weather or not its random
     * or purposeful - a1111 looks the same as z123d3
     *
     *  a1234!ident@host.tld:a2345 <- will get a score of 6
     *
     *  a1234!ident@host.tld:Im Just a User <-- Will get a score of -8
     *                                          and will not match our
     *                                          algorithm.
     *  Alpha = -1
     *  Numerics = +1
     *  All else ignored
     *
     *
     */


	tmp = u->nick;

     //nick
     for (tmp = string; *tmp; tmp++)
     {

	     switch (*tmp)
	     {

	    
             	score +=  OldCharMap[(int) *tmp];
             	score2 +=  CharMap[(int) *tmp];
	     	special_char += SpecialCharMap[(int) *tmp];


	    	tmp++;
             	len++;
	     }
     }
     if (len == 5 || len == 6)
            score++;

	/**
         * this is just bare bones of my algorithm, however we are
         * determining the probability that nick is a bot based on
         * statistical averages and Bayesian properties. 
         * Probability of (A|B) A = Normal B = Bot
         * So the probability that HE is a bot OR that HE is normal.
         * Based on the determining  factors - this formula needs to 
         * to be cleaned up,as its AS old as omega its self, yet seems
         * to hold true 90% of the time.
         */

	nick_count += 1;
	nick_range_val += (float) score2;

	average = (nick_range_val / nick_count);

	if (avg_range == 0)
		avg_range = average;

	alloted_prec = average + (avg_range * 0.05); //allow for a margin of percent between our current score and the average     
	avg_range = alloted_prec;

	/**
	 * Miscellaneous checks
	 * Old style bots have a 6/5 Numeric nick + realname :)
	 * Add this filter into our Bayesian system;.
	 */

	 if (((strlen(u->realname) == 6) || (strlen(u->realname) == 5)) ||
	    ((strlen(u->user) == 6) || (strlen(u->user) == 5))) {
		if (score > 2)
			misc_checks = 1;
	 }


	/**
	 * Add up the Bayesian values
	 */





	total = 0; //total should always = fucking 0 coming into this ;/

	if (score > 2)
		total = 1.0;

	//this check is only worth .5 so it wont tip the scale
	if (score2 > alloted_prec)
		total += 0.5;

	if (special_char >= 3)
		total += 0.75;

	if (misc_checks > 0)
		total += 0.5;

	if (cmd_check)
	{
		sendto_logchan("Scan results for %s",u->nick);

		sendto_logchan("Alpha numeric: %d [Bayesian Value: %f]",score,
						(score > 2)? 1.0 : 0.0);

		sendto_logchan("Probably statistic: %f [Bayesian Value: %f]",score2,
						(score2 > alloted_prec)? 0.5 : 0.0);

		sendto_logchan("Miscellaneous checks: %d [Bayesian Value: %f]", misc_checks,
						(misc_checks > 0)? 0.5 : 0.0);

		sendto_logchan("Total Score: %f",(total)? total : 0.0);
		return;
	}

	HandleResult (u, total);

    return;
}


/********************************************************/

int version_que (int ac, void *nil)
{
	User	*u;
	int		i;
	dlink_node		*dl, *tdl;
	struct scanque_	*sq;

	if (sync_state != 3)
		return EVENT_CONT;

	i = 0;

	DLINK_FOREACH_SAFE (dl, tdl, VersionQ.head)
	{
		if (i > 20)
			return EVENT_CONT;

		i++;

		sq	= dl->data;
		u	= (User *) sq->user;

		sendto_one_privmsg (s_Guardian, u, "\001VERSION\001");

		dlink_delete (dl, &VersionQ);
		dlink_free (dl);
		free (sq);
	}

	DelTimedEvent ("VersionQueue");
	return EVENT_CONT;
}


int scan_que(int ac, void *nil)
{
	User* u;
	struct scanque_ *sq;
	dlink_node *dl,*tdl;
	int found = 0;
	int i = 0;

	//make this repeat until we are synced properly
	if (sync_state != 3)
		return;

	if (has_q)
	{
		sendto_logchan("\002ScanQ:\002 Taking action on nicks found during connection phase");

		DLINK_FOREACH_SAFE(dl,tdl,ScanQ.head)
		{
			if (i > 20) // Only do 20 nicks at a time
				return;

			i++;

			sq = dl->data;

			u = sq->user;

			if ((!u->service) || (!u->oper))
			{
				sendto_logchan("\002Scan:\002 Detected bot %s (%s@%s)",u->nick,u->user,u->host);

			}

			dl = dlink_find_delete(sq,&ScanQ);
			dlink_free(dl);
			free(sq);
		}
	}
	DelTimedEvent("SCANQ");
	return;

}

/********************************************************/

int HandleResult(User* u, int score)
{

	if (!u)
		return 0;
	if (score < 2)
		return 0;

	if (scan_debug)
		sendto_logchan("\002Scan:\002 %s (%s@%s:%s) received a score of %d (System-Average: %f)",u->nick,u->user, u->host, u->realname, score, avg_range);

	write_bl_log(u,score);
	if (sync_state != 3)
	{
		struct scanque_ *sq;

		if (!(sq = (struct scanque_*) malloc(sizeof(struct scanque_))))
			return -1;

		dlink_node *dl = dlink_create();

		sq->user = u;

		dlink_add_tail(sq,dl,&ScanQ);

        has_q = 1;

        return 0;
	}

    sendto_logchan("\002Alert:\002 Possible spam bot detected %s (%s@%s:%s)",u->nick,u->user,u->host,u->realname);
  
   if (scan_action)
    	send_line(":%s KILL %s :%s, You have been flagged as a possible spambot - for any question please email ADMIN_EMAIL",(s_Guardian)? s_Guardian->nick : Me->sid, u->nick, u->nick);

    return MOD_CONT;
}

/*************************************************************/

void write_bl_log(User* u, int score)
{

	fprintf(fp,"BOT: %lu %s (%s@%s:%s) %d %s (%s)\n",(unsigned long) time(NULL), u->nick,u->user,u->host,u->realname,score,u->host,u->serv->name);
	fflush(fp);
	return;
}


/*************************************************************/
static void cmd_version   (User* u, int ac, char **av)
{



	int cnt = 0;
	char *end = NULL;
	char *space = NULL;
	char *params[512];


	if (*av[2] != '\001')
			return;

	*av[2]++;

	if ((end = strchr(av[2],'\001')))
		*end = '\0';

	if ((end = strchr(av[2],' ')))
	{
		end++;
		if (!(space = strchr(end,' ')))
			cnt = 1;
	}
	else
		return;

       
	if (logclients)
       		sendto_logchan("\002Version:\002 %s from %s",end, u->nick);

	if (u->version[0] == '\0')
		strlcpy (u->version, end, sizeof(u->version));

	if (cnt)
		sendto_logchan("\002Alert:\002 Possible litmus trojan detected from \002%s!%s@%s\002", u->nick, u->user, u->host);

       return;
}


/*******************************************************/
static void cmd_quit(User* u, int ac, char **av)
{
	//if we only increment our nick_count and never subtract from it
        //then our average will always be lower so decrement nick_count
        //as they exit the server. Twitch
	nick_count--;
	return;
}



static void cmd_scan(User* u, int ac, char **av)
{

	int n;
	char* params[512];

	User *user;

	if (!ac)
	{
		sendto_one(s_Guardian,u,"Scanner Statistics");
		sendto_one(s_Guardian,u,"Average: %f", avg_range);
		sendto_one(s_Guardian,u,"Total Nick Val: %f",nick_range_val);
		sendto_one(s_Guardian,u,"Nick Count: %d",nick_count);
		sendto_one(s_Guardian,u,"Real Average: %f", (nick_range_val / nick_count));
		return;
	}

	if ((user = find_user(av[0])))
	{
		if (!(user = find_user (av[0])))
		{
			sendto_one(s_Guardian,u,"Unable to find user");
			return;
		}
		scan_nick (2, user);
		return;
	}

	return;
}

void help_scan_ext(User *u) {
        sendto_one(s_Guardian,u,"Syntax: \002SCAN [NICK]\002");
	sendto_one(s_Guardian,u," ");
	sendto_one(s_Guardian,u,"    Scan specified nick, against known bot patterns. If");
	sendto_one(s_Guardian,u,"    no nick is specified then display scanner statistics. ");
	sendto_one(s_Guardian,u," ");
}

void help_scan(User *u) {
	sendto_one_help(s_Guardian,u,"SCAN",   "Perform scan actions on a user.");
}


