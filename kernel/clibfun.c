#include "libfun.h"
#include "publicLib.h"
#include "keyboard.h"
#include "fdreg.h"
char stringDest[200];

//显示字符串
void printString(char*str,int *x,int *y){
    while(*str!=0){
        switch (*str){
            case '\n':
                (*x)=0;
                ++(*y);
                break;
            default:
                printchar(*str,*x,*y);
                ++(*x);
                if((*x)>=80){
                    ++(*y);
                    (*x)=0;
                }
        }
        ++str;
    }
}

void printStringWithIndex(char*str,int *x,int *y){
    while(*str!=0){
        switch (*str){
            case '\n':
                (*x)=0;
                ++(*y);
                setIndex(*x,*y);
                break;
            default:
                printcharWithIndex(*str,*x,*y);
                ++(*x);
                if((*x)>=80){
                    ++(*y);
                    (*x)=0;
                }
        }
        ++str;
    }
}

//设置光标
void setIndex(int x,int y){
    disableInt();
    portOut(0x3D4, 0xE);
    portOut(0x3D5, ((80*y+x)>> 8) & 0xFF);
    portOut(0x3D4, 0xF);
    portOut(0x3D5, (80*y+x) & 0xFF);
    enableInt();
}

//带光标式输出字符
void printcharWithIndex(char a,int x, int y){
    printchar(a,x,y);
    ++x;
    if(x>=80){
        x=0;
        ++y;
    }
    setIndex(x,y);
}

char _getch_(){
    while(1){
        struct keyGet kg=keyboardRead();
        if(kg.isGet==1){
            switch(kg.c){
                case ENTER:
                    return'\n';
                default:
                    return (char)kg.c&0xff;
            }
        }
    }
}

int processRun(int enterAddr){
    disableInt();
    readyPcbNormalAdd(0x08,enterAddr,0x10);
    enableInt();
    return 0;
}

unsigned short _getCluster(int pos,unsigned char* fat1){
         if(pos%2==0){
            unsigned short a=(unsigned short)(*((unsigned char*)(fat1+pos/2*3)));
            unsigned short b=(((unsigned short)*((unsigned char*)(fat1+pos/2*3+1)))&0x0f)<<8;
            return a|b;
        }else{
            unsigned short a=(unsigned short)(*((unsigned char*)(fat1+pos/2*3+2)))<<4;
            unsigned short b=(((unsigned short)*((unsigned char*)(fat1+pos/2*3+1)))>> 4);
            return a|b;
        }
}

void _setCluster(int pos,unsigned char* fat1,unsigned short cluster){
    if(pos%2==0){
        unsigned char a=cluster&0x0ff;
        unsigned char b=(cluster>>8)&0x0f;
        *((unsigned char*)(fat1+pos/2*3)) = a;
        *((unsigned char*)(fat1+pos/2*3+1)) &= 0xf0;
        *((unsigned char*)(fat1+pos/2*3+1)) |= b;
    }else{
        unsigned char a=cluster&0x0f;
        unsigned char b=(cluster>>4)&0x0ff;
        *((unsigned char*)(fat1+pos/2*3+1)) &= 0x0f;
        *((unsigned char*)(fat1+pos/2*3+1)) |= a;
        *((unsigned char*)(fat1+pos/2*3+2)) =b;
    }
}

void _clearCluster(int cluster,unsigned char* fat1){
    int curcluster=cluster;
    while(curcluster!=0xfff){
        print(itoa(curcluster,0,10));
        int tmp=_getCluster(curcluster,fat1);
        _setCluster(curcluster,fat1,0);
        curcluster=tmp;
    }

    _setCluster(cluster,fat1,0xfff);
}

int fileCreate(char*name){
    unsigned char* fat1=(unsigned char*)0x2000;
    unsigned char* root=(unsigned char*)0x4000;
    for (int i = 0; i < 9; ++i)
    {
        floppyReadSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,root+i*512);
    }
    int freeFat=0;
    for (int i = 2; ; ++i)
    {
        if(_getCluster(i,fat1)==0){
            freeFat=i;
            _setCluster(freeFat,fat1,0xfff);
            break;
        }
    }
    for (int i = 0; i<14*512/32; ++i)
    {
        unsigned short cluster=*((unsigned short*)(root+i*32+32-6));
        if(cluster==0){
            myMemset(root+i*32,0,32);
            *((unsigned short*)(root+i*32+32-6))=freeFat;
            *((unsigned int*)(root+i*32+32-4))=0;
            memcpy(root+i*32,name,11);
            break;
        }
    }
    for (int i = 0; i < 9; ++i)
    {
        floppyWriteSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyWriteSector(i+19,root+i*512);
    }
}


unsigned int fileRead(char*name,void*buff){
    unsigned char* fat1=(unsigned char*)0x2000;
    unsigned char* root=(unsigned char*)0x4000;
    unsigned char*tmp_buff=(unsigned char*)0x1000;
    for (int i = 0; i < 9; ++i)
    {
        floppyReadSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,root+i*512);
    }

    unsigned short cluster;
    unsigned int size;
    int finded=0;
    for (int i = 0;i<14*512/32 ; ++i)
    {
        cluster=*((unsigned short*)(root+i*32+32-6));
        char tmp[20];
        myMemset(tmp,0,20);
        if(cluster!=0){
            memcpy(tmp,root+i*32,11);
            if(strcmp(tmp,name)==0){
                size=*((unsigned int*)(root+i*32+32-4));
                finded=1;
                break;
            }
        }
    }
    if(finded==0)
        return 0xffffffff;
    
    unsigned int fileSize=size;
    int readed=0;
    while(size>0){
        int to_read=size<512?size:512;
        size-=to_read;
        floppyReadSector(33-2+cluster,tmp_buff);
        memcpy(buff+readed,tmp_buff,to_read);
        readed+=to_read;
        cluster=_getCluster(cluster,fat1);
    }
    return  fileSize;
}

int fileWrite(char*name,void*buff,unsigned int fileSize){
    unsigned char* fat1=(unsigned char*)0x2000;
    unsigned char* root=(unsigned char*)0x4000;
    for (int i = 0; i < 9; ++i)
    {
        floppyReadSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,root+i*512);
    }

    unsigned short cluster;
    unsigned int size;
    int finded=0;
    for (int i = 0;i<14*512/32 ; ++i)
    {
        cluster=*((unsigned short*)(root+i*32+32-6));
        char tmp[20];
        myMemset(tmp,0,20);
        if(cluster!=0){
            memcpy(tmp,root+i*32,11);
            if(strcmp(tmp,name)==0){
                *((unsigned int*)(root+i*32+32-4))=fileSize;
                finded=1;
                break;
            }
        }
    }
    if(finded==0){
        fileCreate(name);
    }

    // _clearCluster(cluster,fat1);
    // print(itoa(cluster,0,10));
    unsigned int writed=0;
    unsigned int fileRest=fileSize;
    while(fileRest>0){
        unsigned int to_write=fileRest<512?fileRest:512;
        floppyWriteSector(33-2+cluster,buff+writed);
        writed+=to_write;
        fileRest-=to_write;
        if(fileRest>0){
            int freeFat;
            for (int i = 2; ; ++i)
            {
                if(_getCluster(i,fat1)==0){
                    _setCluster(i,fat1,0xfff);
                    freeFat=i;
                    break;
                }
            }
            cluster=freeFat;
        }
    }

    for (int i = 0; i < 9; ++i)
    {
        floppyWriteSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyWriteSector(i+19,root+i*512);
    }
    return 1;
}