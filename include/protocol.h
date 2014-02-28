#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__



struct { 
  int (*client)(char * nick);
} protocol;


#endif //__PROTOCOL_H__