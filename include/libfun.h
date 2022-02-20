#ifndef _LIBFUN_H_
#define _LIBFUN_H_
//外部函数
int readyPcbNormalAdd(int cs,int eip,int ds);
//读键盘
struct keyGet keyboardRead();
//cfun

void printString(char*str,int *x,int *y);
void printStringWithIndex(char*str,int *x,int *y);
void printcharWithIndex(char a,int x, int y);
void setIndex(int x,int y);
char _getch_();
int processRun(int enterAddr);
//nasm fun
int loadProgramElf(int i);//i从1开始
void portOut(unsigned short port, unsigned char num );
unsigned char portIn(unsigned short port);
void printchar(char a,int x, int y);
int getIDTAddr();
void disableInt();
void enableInt();

//file
void _setCluster(int pos,unsigned char* fat1,unsigned short cluster);
void _clearCluster(int cluster,unsigned char* fat1);
unsigned short _getCluster(int pos,unsigned char* fat1);


int fileCreate(char*name);
unsigned int fileRead(char*name,void*buff);
int fileWrite(char*name,void*buff,unsigned int fileSize);
#endif