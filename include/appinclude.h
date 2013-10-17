#ifndef __APPINCLUDE_H__
#define __APPINCLUDE_H__



#define MAXHOST 255
#define MAXPATH 255
#define MAXPASS 255
#define MAXUSER 100

#define MAXLEN 2048
#define MAXREADBUFFER MAXLEN*120+5


#define SRC_DIR PREFIX "/src/"
#define OMG_PKGFULL OMG_PKGNAME "(" OMG_RCNAME ")"

#define _A_ __FILE__, __LINE__, __PRETTY_FUNCTION__

#ifndef E 
#	define E 
#endif

#include "version.h"

#include "dlink.h"
#include "memory.h"

#include "sighandler.h"
#include "logger.h"
#include "tools.h" 
#include "thread.h"
#include "module.h"
#include "database.h"
#include "configparser.h"
#include "socket.h"
#include "core.h"
#include "socketengine.h"




#endif