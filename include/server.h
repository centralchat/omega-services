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
 *    $Id: server.h 2396 2012-06-30 16:53:15Z twitch $
 */

#include "setup.h"
#include "version.h"
#include "configparse.h"
#include "core.h"
#include "threads.h"
#include "irc_string.h"
#include "user.h"
#include "channel.h"
#include "access.h"
#include "socketengine.h"
#include "module.h"
#include "links.h"
#include "protocol.h"
#include "match.h"
#include "eventhandler.h"
#include "cmds.h"
#include "tools.h"
#include "log.h"
#include "dbmanager.h"
#include "metadata.h"
#include "send.h"

dlink_list  servlist;
dlink_list  ax_list;

