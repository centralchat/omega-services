#ifndef ___LOGGER_H___
#define ___LOGGER_H___


/***
 * Log a message, attributes will be provided from the LOG_TYPES definition
 */

void log_message(int level, const char *file, int line, const char *function, const char *fmt, ...)
	__attribute__((format(printf, 5, 6)));

/**
 * alog macro provided for backwards compatibility
 */

#define alog log_message 


#define _LOG_INFO     0
#define _LOG_WARNING  1
#define _LOG_NOTICE   2
#define _LOG_ERROR    3
#define _LOG_CRITICAL 4
#define _LOG_FATAL    5
#define _LOG_DEBUG    6
#define _LOG_DEBUG2   7
#define _LOG_DEBUG3   8
#define _LOG_MODULE   9
#define _LOG_CONFIG   10
#define _LOG_DATABASE 11
#define _LOG_SOCKET   12

#define LOG_INFO     _LOG_INFO    , _A_
#define LOG_WARNING  _LOG_WARNING , _A_
#define LOG_NOTICE   _LOG_NOTICE  , _A_
#define LOG_ERROR    _LOG_ERROR   , _A_
#define LOG_CRITICAL _LOG_CRITICAL, _A_
#define LOG_FATAL    _LOG_FATAL   , _A_
#define LOG_DEBUG    _LOG_DEBUG   , _A_
#define LOG_DEBUG2   _LOG_DEBUG2  , _A_
#define LOG_DEBUG3   _LOG_DEBUG3  , _A_
#define LOG_MODULE   _LOG_MODULE  , _A_
#define LOG_CONFIG   _LOG_CONFIG  , _A_
#define LOG_DATABASE _LOG_DATABASE, _A_
#define LOG_SOCKET   _LOG_SOCKET  , _A_


E int  log_open();
E void log_close(void);

E char log_name[1024];


#endif