/*
 *                OMEGA IRC SECURITY SERVICES
 *                  (C) 2008-2012 Omega Dev Team
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
 *    $Id: cmds.h 2368 2012-02-27 09:09:10Z twitch $
 */


#ifndef __CMD_H__
#define __CMD_H__

#define CMD_ERR_OK      0
#define CMD_ERR_INV     1
#define CMD_ERR_ACC     2
#define CMD_ERR_PARAMS  3
#define CMD_ERR_INDEX   4
#define CMD_ERR_HASH    5

#define CMD_ADD_HEAD 1
#define CMD_ADD_TAIL 0

#define CMD_HASH_KEY 1024



typedef struct cmdbase_ {
	char name[50]; 
	char *module; //The module the command is associated with.
	char access[12]; //we only support flags
	int hashlp;
	int hashlpext;
	int hascmd;
	int operonly;
	int log;
	unsigned int entrycount;
	void (*fnct)(User*, int, char**);
	void (*ext_help)(User*);
} CmdBase;


struct cmdhash_ {
	char name[30]; 
	dlink_list commands;
	dlink_list command_help_shrt;
	int entry_cnt;
};

typedef struct help_entry {
	void (*routine)(User*);
} HelpEntry;

CmdHash *UserCmdHash[1024];    //This users CMDhash :)
dlink_list cmd_help[1024];

int HELP_FOREACH(User*, User*);
int do_cmd(char *, User *, User *, int, char **);

void ACCESS_DENIED(User*,User*) ;

CmdHash* CmdHashCreate(User *u);

int  AddCmd (User *, char *,char *, void (*fnct)(User*, int, char**), void (*hlp)(User*), int);
void DelCmd (User *, char *, void (*fnct)(User*, int, char**), void (*hlp)(User*));

int AddHelp (User* u, void (*helpcmd)(User*));
int DelHelp (User *u, void (*helpcmd)(User*));


char *CmdErr   (int);

static void ClientCmdNull(User *u,char *av);

/******************************************************/
/**
 * Depreciated Stuff
 */


#endif //__CMD_H__
