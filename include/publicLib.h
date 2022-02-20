#ifndef _PUBLICLIB_H_

void test();
//input and output
void putch(char c);
char getch();
void putstr(char*str);
void print(char*str);
char* getstr(char*str);
void clearScrean();

//string.h
void _printchar_(char a,int x, int y);
//匹配前缀，匹配返回1，不匹配返回0
int strMatchPrefix(const char*prefix,const char *str);
char *itoa(int value, char *string,int radix);
char* strrev(char*src);
int strlen(const char*str);
char* strcpy(char*des,const char*src);
int strcmp(const char*a,const char*b);

void *memcpy(void *destin, const void *source, unsigned n);
void myMemset(void *s,int ch,int n);

unsigned int getTime();
int myFork();
void myExit();
void mySleep(int t);
void myWait(int id);

//sync
int getSem(int val);
int freeSem(int semId);
int P(int semId);
int V(int semId); 

// file
unsigned char* readDisk(int sectNo);

// syscall
void int20h();
//int21h 0是getchar,1是cls，2是getTime,3是doFork
//4是doExit, 5是doSleep, 6是doWait
//101是getsem，102是freesem，103是P，104是V
//200是readDisk(int 扇区),read到0x3000
int int21h(int choose,int arg1); 
void int22h(); //转移到下一个进程
void int23h();

#define _PUBLICLIB_H_
#endif