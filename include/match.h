

#ifndef __MATCH_H__
#define __MATCH_H__



int match_regx(char*,char*);

int match_wild(const char *pattern, const char *str, int docase);


int match_cidr(unsigned char addr[16], unsigned char mask[16], unsigned char bits);


#endif //__MATCH_H__
