#ifndef __METADATA_H__
#define __METADATA_H__


/******************************************/
/** 
 *  Metadata
 * 
 * This struct stores entity meta data
 * 
 */

typedef struct { 
  char key[100];
  char value[256];
} Metadata; 


Metadata *get_metadata_user(User *, char *);
int add_metadata_user(User *, char *, char *);
int add_metadata_channel(Channel*, char*, char *);

#endif //___METADATA_H__
