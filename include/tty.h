#ifndef _TTY_H_
#define _TTY_H_ 
#define OrderMaxLen 70
#define OrderHistoryNum 15
void ttyShow();
void execOrder(char*order);
int strcmp(const char*a,const char*b);
void execFun(char *exe);
void dir();
void del(char *str);
void cp(char * src,char *dst);

#endif