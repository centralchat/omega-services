
#ifndef __INSPIRCD_H__
#define __INSPIRCD_H__



void PtclFactory();
int insp_connect(char *source, char *trg, char *params);
int insp_fjoin(char *source, char *trg, char *user);
int insp_oper(char *source, char *opertype, char *params);
int insp_ping(char *source, ...);

#endif //__INSPIRCD_H__
