#include "publicLib.h"
//input and output
int x=0,y=0;
void test(){
    int *addr=&x;
    putstr(itoa((int)addr,0,16));
}
void putch(char c){
    if(c=='\n'){
        x=0;
        ++y;
    }else{
        if(x>=80){
            ++y;
            x=0;
        } 
        if(y>=25){
            y=0;
        }
        _printchar_(c,x,y);
        ++x;
    }
}

char getch(){
    char c=int21h(0,0);
    putch(c);
    return c;
}

void putstr(char*str){
    int len=strlen(str);
    for (int i = 0; i < len; ++i)
    {
        putch(str[i]);
    }
    ++y;
    x=0;
    if(y==25){
        y=0;
    }
}

char* getstr(char*str){
    char c;
    int n=0;
    while((c=getch())!='\n'){
        str[n++]=c;
    }
    str[n]=0;
    return str;
}

void print(char*str){
    int len=strlen(str);
    for (int i = 0; i < len; ++i)
    {
        putch(str[i]);
    }
}

//清除屏幕
void clearScrean(){
    x=0;y=0;
    for (int i=0; i < 80*25; ++i)
    {
        putch(' ');
    }
}

//string.h------------------------------------
//匹配前缀，匹配返回1，不匹配返回0
int strMatchPrefix(const char*prefix,const char *str){
    while((*prefix)!=0){
        if((*str)!=(*prefix))
            return 0;
        ++str;
        ++prefix;
    }
    return 1;
}

//itoa
char stringTmp[200];
char *itoa(int value, char *string,int radix){
    if(string==0) string=stringTmp;
    if(value==0){
        string[0]='0';
        string[1]=0;
        return string;  
    } 
    int index=0;
    while(value>0){
        char add=value%radix;
        value/=radix;
        if(add<10){
            add+='0';
        }else{

            add+='a'-10;
        }
        string[index++]=add;
    }
    string[index]=0;
    strrev(string);
    return string;
}

//strrev
char* strrev(char*src){
    char*a=src;
    char*b=src;
    int max=0x00ffffff;
    while(*b != 0 && max!=0){
        --max;
        ++b;
    }
    --b;
    while(b-a>0){
        char tmp=*a;
        *a=*b;
        *b=tmp;
        ++a;--b;
    }
    return src;
}

//strlen
int strlen(const char*str){
    int cnt=0;
    while(*str!=0){
        ++cnt;
        ++str;
        if(cnt>0x00ffffff)
            return -1;
    }
    return cnt;
}

//strcpy
char* strcpy(char*des,const char*src){
    char*tdes=des;
    int maxLen=0x00ffffff;
    while(*src!=0){
        --maxLen;
        if(maxLen==0)
            return 0;
        *des=*src;
        ++des;++src;
    }
    *des=0;
    return tdes;
}

//strcmp
int strcmp(const char*a,const char*b){
    while((*a)!=0||(*b)!=0){
        if((*a)==0)
            return -1;
        else if((*b)==0)
            return 1;
        else if((*a)!=(*b))
            return (*a)-(*b);
        ++a;
        ++b;
    }
    return 0;
}

void* memcpy(void*dest,const void*source,unsigned n){
    for (int i = 0; i < n; ++i)
    {
        ((char*)dest)[i]=((const char*)source)[i];
    }
}

void myMemset(void *s,int ch,int n){
    char c=ch;
    for (int i = 0; i < n; ++i)
    {
        memcpy(s+i,&c,1);
    }
}

unsigned int getTime(){
    return int21h(2,0);
}

// int myFork(){
//     int a=int21h(3,0);
//     return a;
// }

void myExit(){
    int21h(4,0);
    return;
}

void mySleep(int t){
    int time=t/10;
    int21h(5,time);
    return;
}

void myWait(int id){
    int21h(6,id);
    return;
}


//sync
int getSem(int val){return int21h(101,val);}
int freeSem(int semId){return int21h(102,semId);}
int P(int semId){return int21h(103,semId);}
int V(int semId){return int21h(104,semId);}

// file
unsigned char* readDisk(int sectNo){
    int21h(200,sectNo);
    return (unsigned char*)0x3000;
}