#include "tty.h"
#include "libfun.h"
#include "publicLib.h"
#include "keyboard.h"
#include "fdreg.h"
extern unsigned int clock;

int Xnow=0,Ynow=0;
void ttyShow(){
    // floppyReadSector(19,(unsigned char *)0x3000);
    // floppyReadSector(2,(unsigned char *)0x3000);
    // floppyReadSector(4,(unsigned char *)0x3000);
    // floppyReadSector(11,(unsigned char *)0x3000);
    // floppyReadSector(2,(unsigned char *)0x3000);
    clearScrean();
    Xnow=0,Ynow=0;
    char order[OrderMaxLen];
    int orderIndex=0;
    // int orderLen=0;
    char orderHistory[OrderHistoryNum][OrderMaxLen];
    int orderHistorySelect=0;
    int orderHistoryLower=0;
    int orderHistoryUpper=0;
    printString("welcome to pxj's OS\n",&Xnow,&Ynow);
    while(1){
        if(Ynow>=25){
            clearScrean();
            Xnow=0,Ynow=0;
        }
        printStringWithIndex(">>",&Xnow,&Ynow);
        orderIndex=0;
        // orderLen=0;
        orderHistorySelect=orderHistoryUpper;
        int nextOrder=0;
        while(!nextOrder){
            struct keyGet kg=keyboardRead();
            if(kg.isGet==1){
                switch (kg.c){
                    case BACKSPACE:
                        if(orderIndex>0){
                            --Xnow;
                            if(Xnow<0){
                                Xnow=79;
                                --Ynow;
                            }
                            printchar(' ',Xnow,Ynow);
                            setIndex(Xnow,Ynow);
                            
                            orderIndex--;
                        }
                        break;
                    case ENTER:
                        order[orderIndex]=0;
                        strcpy(orderHistory[orderHistoryUpper],order);
                        orderHistoryUpper=(orderHistoryUpper+1)%OrderHistoryNum;
                        if(orderHistoryUpper==orderHistoryLower)
                            orderHistoryLower=(orderHistoryLower+1)%OrderHistoryNum;
                        execOrder(order);
                        nextOrder=1;
                        break;
                    case PAD_UP:
                        if(orderHistorySelect!=orderHistoryLower){
                            for (int i = 0; i < OrderMaxLen; ++i)
                            {
                                printchar(' ',i+2,Ynow);
                            }
                            Xnow=2;
                            orderHistorySelect=(orderHistorySelect-1+OrderHistoryNum)%OrderHistoryNum;
                            printStringWithIndex(orderHistory[orderHistorySelect],&Xnow,&Ynow);
                            strcpy(order,orderHistory[orderHistorySelect]);
                            orderIndex=strlen(orderHistory[orderHistorySelect]);
                        }
                        break;
                    case PAD_DOWN:
                        if(orderHistorySelect!=orderHistoryUpper&& \
                            (orderHistorySelect+1)%OrderHistoryNum!=orderHistoryUpper){
                            for (int i = 0; i < OrderMaxLen; ++i)
                            {
                                printchar(' ',i+2,Ynow);
                            }
                            Xnow=2;
                            orderHistorySelect=(orderHistorySelect+1)%OrderHistoryNum;
                            printStringWithIndex(orderHistory[orderHistorySelect],&Xnow,&Ynow);
                            strcpy(order,orderHistory[orderHistorySelect]);
                            orderIndex=strlen(order);
                        }
                        break;
                    case PAD_LEFT:
                    case PAD_RIGHT:
                    default:
                        if(!(kg.c & FLAG_EXT)){
                            if(orderIndex>=OrderMaxLen)
                                break;
                            printcharWithIndex((char)kg.c&0xff,Xnow,Ynow);
                            ++Xnow;
                            if(Xnow>=80){Xnow=0;Ynow++;}
                            order[orderIndex++]=kg.c;
                        }
                }
            }
        }
    }
}

void execOrder(char*order){
    printString("\n",&Xnow,&Ynow);
    // printString(order,&Xnow,&Ynow);
    
    if(strcmp(order,"cls")==0){
        clearScrean();
        Xnow=0,Ynow=0;
    }
    else if(strcmp(order,"showTimer")==0){
        char string[200];
        printString("the system clock is:",&Xnow,&Ynow);
        printString(itoa(clock,string,10),&Xnow,&Ynow);
        printString("\n",&Xnow,&Ynow);
    }
    else if(strMatchPrefix("exec",order)){
        execFun(order+5);
    }
    else if(strMatchPrefix("print ",order)){
        printString(order+strlen("print "),&Xnow,&Ynow);
        printString("\n",&Xnow,&Ynow);
    }
    else if(strcmp(order,"dir")==0){
        dir();
        printString("\n",&Xnow,&Ynow);
    }
    else if(strMatchPrefix("del ",order)){
        // del("LOADER  BIN");
        order[11+4]=0;
        del(order+4);
        printString("\n",&Xnow,&Ynow);
    }
    else if(strMatchPrefix("cp ",order)){
        order[3+11]=0;
        order[3+11+1+11]=0;
        cp(order+3,order+3+11+1);
        printString("\n",&Xnow,&Ynow);
    }
    else{
        printStringWithIndex("can't understanding your order\n",&Xnow,&Ynow);
    }
}

#include"testFun.h"
void execFun(char *exe){
    if(exe[0]>'0' &&exe[0]<='3'){
        clearScrean();
        Xnow=0,Ynow=0;
        int num=exe[0]-'0';
        int enterAddr=loadProgramElf(num);
        if(num==3){
            testFun();
        }else{
            processRun(enterAddr);
        }
        
        // myMemset((void*)enterAddr,0,1024*32);
    }
    else{
        printStringWithIndex("invalid program\n",&Xnow,&Ynow);
    }
}


void dir(){
    unsigned char* addr=(unsigned char * )0x3000;
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,addr+i*512);
    }
    for (int i = 0;i<14*512/32 ; ++i)
    {
        unsigned short cluster=*((unsigned short*)(addr+i*32+32-6));
        char tmp[20];
        myMemset(tmp,0,20);
        if(cluster!=0){
            memcpy(tmp,addr+i*32,11);
            printString(tmp,&Xnow,&Ynow);
            printString("\n",&Xnow,&Ynow);
        }
    }
    print("\n");

}

void del(char *str){
    unsigned char* fat1=(unsigned char*)0x2000;
    unsigned char* addr=(unsigned char * )0x4000;
    for (int i = 0; i < 9; ++i)
    {
        floppyReadSector(i+1,fat1+i*512);
    }
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,addr+i*512);
    }
    for (int i = 0;i<14*512/32 ; ++i)
    {
        unsigned short cluster=*((unsigned short*)(addr+i*32+32-6));
        char tmp[20];
        myMemset(tmp,0,20);
        if(cluster!=0){
            memcpy(tmp,addr+i*32,11);
            if(strcmp(tmp,str)==0){
                char t_buf[32];
                myMemset(t_buf,0,32);
                memcpy(addr+i*32,t_buf,32);
                _clearCluster(cluster,fat1);
                _setCluster(cluster,fat1,0);
                for (int i = 0; i < 14; ++i)
                {
                    floppyWriteSector(i+19,addr+i*512);
                }
                for (int i = 0; i < 9; ++i)
                {
                    floppyWriteSector(i+1,fat1+i*512);
                }
                printString("del ok\n",&Xnow,&Ynow);
                return;
            }

        }

    }
    printString("del fail\n",&Xnow,&Ynow);
}

void cp(char * src,char *dst){
    unsigned char* addr=(unsigned char * )0x3000;
    for (int i = 0; i < 14; ++i)
    {
        floppyReadSector(i+19,addr+i*512);
    }
    for (int i = 0;i<14*512/32 ; ++i)
    {
        unsigned short cluster=*((unsigned short*)(addr+i*32+32-6));
        char tmp[20];
        myMemset(tmp,0,20);
        if(cluster!=0){
            memcpy(tmp,addr+i*32,11);
            if(strcmp(tmp,src)==0){
                for (int j = 0;j<14*512/32 ; ++j){
                    unsigned short t_cluster=*((unsigned short*)(addr+j*32+32-6));
                    if(t_cluster==0){
                        memcpy(addr+j*32,addr+i*32,32);
                        memcpy(addr+j*32,dst,11);
                        for (int i = 0; i < 14; ++i)
                        {
                            floppyWriteSector(i+19,addr+i*512);
                        }
                        printString("cp ok\n",&Xnow,&Ynow);
                        return;
                    }
                }
            }

        }

    }
    printString("cp fail\n",&Xnow,&Ynow);
    // char buf[2000];
    // unsigned int a=fileRead(src,buf);
    // if(a==0xffffffff){
    //     printString("cp fail\n",&Xnow,&Ynow);
    //     return;
    // }
    // fileWrite(dst,buf,a);
    // printString("cp ok\n",&Xnow,&Ynow);
}

