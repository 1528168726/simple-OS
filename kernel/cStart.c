#include "cStart.h"
#include "libfun.h"
#include "tty.h"
#include "publicLib.h"
#include "sync.h"
#include "fdreg.h"

CPUreg cpuReg[20];
PCB pcbs[PCBSNUMBER]={0};
int sleepTimer[TIMERMAX][2];
int curPpcb=-1;
int blockedPpcb=-1;
int stackBaseStatus=0;

void cStart(){
    disableInt();
    {//初始化
        stackBaseStatus=0;
        myMemset(pcbs,0,sizeof(pcbs));
        myMemset(sleepTimer,0,sizeof(sleepTimer));
        syncInitialize();
    }
    clearScrean();
    readyPcbNormalAdd(0x08,(int)ttyShow,0x10);
    int22h();

    return;
}

void Init8259A(){
    portOut(0x20,0x11);
    portOut(0xa0,0x11);
    portOut(0x21,startInterruptVector);
    portOut(0xa1,startInterruptVector+0x08);
    portOut(0x21,0x04);
    portOut(0xa1,0x02);
    portOut(0x21,0x01);
    portOut(0xa1,0x01);

    portOut(0x21,0xbc);//开启一些主8259中断
    portOut(0xa1,0xff);//屏蔽所有从8259中断
}

//设置时钟频率为100hz
void setTimebreak(){
    portOut(0x43,0x34);
    portOut(0x40,(unsigned char)(1193182L/100));
    portOut(0x40,(unsigned char)((1193182L/100)>>8));
}


//vector 指中断向量号，handleFun 指处理该中断的函数地址，attribute指门的类型
void initIdtDesc(char vector, int handleFun,int attribute)
{
    GATE* tmpGate=(GATE*)getIDTAddr()+vector; 
    tmpGate->offset_low = handleFun & 0xFFFF;
    tmpGate->selector=0x08;
    tmpGate->attribute=attribute;
    tmpGate->offset_high = (handleFun >> 16) & 0xFFFF;
}
void tmp(){
    return;
}
void setIDT(){
    for (int i = 0; i <= 0x19; ++i)
    {
        initIdtDesc(i,(int)tmp,IGATE);
    }

    setTimebreak();
    initIdtDesc(startInterruptVector+0,(int)_ClockHandler,IGATE);
    initKeyboard();
    initIdtDesc(startInterruptVector+1,(int)_keyboardHandler,IGATE);
    initIdtDesc(startInterruptVector+6,(int)floppy_reset_handler,IGATE);
    initIdtDesc(0x20,(int)_Int20h,IGATE);
    initIdtDesc(0x21,(int)_Int21h,IGATE);
    initIdtDesc(0x22,(int)_Int22h,IGATE);
}

unsigned int clock=0;
int _getTime_(){return clock;}
void ClockHandler(){
    ++clock;
    
    //风火轮
    if(clock%32==0){ 
       hotWheels(clock);
    }

    timerFunc();

    if(curPpcb!=-1){
        memcpy(&(pcbs[curPpcb].cpureg),cpuReg,sizeof(CPUreg));
        curPpcb=pcbs[curPpcb].Pnext;
        memcpy(cpuReg,&(pcbs[curPpcb].cpureg),sizeof(CPUreg));
    }
}

int readyPcbAdd(CPUreg* cpureg){
    int newPpcb=getFreePcb();
    if(newPpcb<0 || newPpcb>=PCBSNUMBER)
        return 0;
    int stackBase=getNewStack();
    cpureg->ss=0x10;
    cpureg->esp=stackBase;
    setPcb(newPpcb,cpureg,"default",stackBase,NOFATHER);
    _pcbListInsert(&curPpcb,&newPpcb);
    return 1;
}

int readyPcbNormalAdd(int cs,int eip,int ds){
    CPUreg tmp;
    tmp.cs=cs;
    tmp.eip=eip;
    tmp.ds=ds;tmp.es=ds;tmp.fs=ds;
    tmp.gs=0x001b;
    //tmp.gs=ds, 一些错误代码
    tmp.eflags=0x00000202; //除了中断使能设为1，其他为0。
    return readyPcbAdd(&tmp);
    // while(1);
}



void blockPcb(int Ppcb){
    pcbs[Ppcb].isBlocked=1;
    int tmp=_pcbListDelete(&curPpcb,&Ppcb);
    _pcbListInsert(&blockedPpcb,&Ppcb);
    if(tmp==-1){
        while(1);
    }
}

void wakeupPcb(int Ppcb){
    pcbs[Ppcb].isBlocked=0;
    _pcbListDelete(&blockedPpcb,&Ppcb);
    _pcbListInsert(&curPpcb,&Ppcb);
}

int doFork(){
    //保存
    memcpy(&(pcbs[curPpcb].cpureg),cpuReg,sizeof(CPUreg));

    int Ppcb=curPpcb;
    int newPpcb=getFreePcb();
    if(newPpcb==-1)
        return -1;
    int stackAddr=getNewStack();
    if(stackAddr==-1)
        return -1;
    int stackSize=pcbs[Ppcb].stackBase-pcbs[Ppcb].cpureg.esp;
    //复制pcb
    setPcb(newPpcb,&pcbs[Ppcb].cpureg,"default",stackAddr,Ppcb);
    //设置栈指针并复制栈
    pcbs[newPpcb].cpureg.esp=stackAddr-stackSize;
    pcbs[newPpcb].cpureg.ebp=pcbs[newPpcb].cpureg.esp+(pcbs[Ppcb].cpureg.ebp-pcbs[Ppcb].cpureg.esp);
    memcpy((void*)pcbs[newPpcb].cpureg.esp,(void*)pcbs[Ppcb].cpureg.esp,stackSize);
    //两个线程各自的返回值
    pcbs[newPpcb].cpureg.eax=0;
    pcbs[Ppcb].cpureg.eax=pcbs[newPpcb].id;
    _pcbListInsert(&curPpcb,&newPpcb);
    // while(1);
    //恢复
    int22h();
}

void doExit(){
    freeStack(pcbs[curPpcb].stackBase);
    pcbs[curPpcb].used=0;
    // print("yes\n");

    if(pcbs[curPpcb].fatherPpcb!=NOFATHER){
        //如果父线程被阻塞，唤醒
        int fatherPpcb=pcbs[curPpcb].fatherPpcb;
        if(pcbs[fatherPpcb].isBlocked){
            wakeupPcb(fatherPpcb);
        }
    }
    if(_pcbListDelete(&curPpcb,&curPpcb)==-1)
        while(1);
    int22h();
}

void doSleep(int t){
    //保存
    memcpy(&(pcbs[curPpcb].cpureg),cpuReg,sizeof(CPUreg));

    for (int i = 0; i < TIMERMAX; ++i)
    {
        if(sleepTimer[i][0]==0){
            sleepTimer[i][0]=t;
            sleepTimer[i][1]=curPpcb;
            blockPcb(curPpcb);
            //恢复
            int22h();
        }
    }
    return;
}

void doWait(int id){
    int Ppcb=id^IDOFFSET;
    if(pcbs[Ppcb].used){
        memcpy(&(pcbs[curPpcb].cpureg),cpuReg,sizeof(CPUreg));
        blockPcb(curPpcb);
        int22h();
    }
}

int _pcbListDelete(int*Ppcb,int* toDeletePpcb){
    if(pcbs[*Ppcb].Pnext==*Ppcb && *Ppcb==* toDeletePpcb){
        *Ppcb=-1;
        return -1;
    }else if(* toDeletePpcb == *Ppcb){
        int pre=pcbs[*Ppcb].Ppre;
        int next=pcbs[*Ppcb].Pnext;
        pcbs[pre].Pnext=next;
        pcbs[next].Ppre=pre;
        *Ppcb=next;
        return next;
    }else{
        int pre=pcbs[* toDeletePpcb].Ppre;
        int next=pcbs[* toDeletePpcb].Pnext;
        pcbs[pre].Pnext=next;
        pcbs[next].Ppre=pre;
        return *Ppcb;
    }
}

int _pcbListInsert(int*Ppcb,int* toInsertPpcb){
    if(*Ppcb==-1){
        *Ppcb=* toInsertPpcb;
        pcbs[*Ppcb].Pnext=*Ppcb;
        pcbs[*Ppcb].Ppre=*Ppcb;
    }else if(*Ppcb>=0){
        pcbs[*toInsertPpcb].Pnext=pcbs[*Ppcb].Pnext;
        pcbs[*toInsertPpcb].Ppre=*Ppcb;

        int tmp=pcbs[*Ppcb].Pnext;
        pcbs[tmp].Ppre=*toInsertPpcb;
        pcbs[*Ppcb].Pnext=*toInsertPpcb;
    }
    return *Ppcb;
}

int getFreePcb(){
    for (int i = 0; i < PCBSNUMBER; ++i)
    {
        if(pcbs[i].used==0)
            return i;
    }
    return -1;
}

void setPcb(int Ppcb,const CPUreg* cpureg,char*name,int stackBase,int fatherPpcb){
    pcbs[Ppcb].id=Ppcb^IDOFFSET;
    pcbs[Ppcb].used=1;
    pcbs[Ppcb].isBlocked=0;
    pcbs[Ppcb].stackBase=stackBase;
    pcbs[Ppcb].fatherPpcb=fatherPpcb;
    if(strlen(name) < sizeof(pcbs[0].name)){
        strcpy(pcbs[Ppcb].name,name);
    }else{
        strcpy("name is too long",name);
    }
    memcpy(&(pcbs[Ppcb].cpureg),cpureg,sizeof(CPUreg));
}

int getNewStack(){
    for (int i = 0; i < 16; ++i)
    {
        if( (stackBaseStatus&(1<<i)) ==0 ){
            stackBaseStatus=stackBaseStatus^(1<<i);
            return STACKPOSITION+0x10000*i;
        }
    }
    return -1;
}

void freeStack(int stackBase){
    int i=(stackBase-STACKPOSITION)/0x10000;
    if((stackBaseStatus&(1<<i)) !=0)
        stackBaseStatus=stackBaseStatus^(1<<i);
}



void hotWheels(int clock){
    unsigned int choose=(clock/32)%8;
    printchar('o',75,22);
    switch (choose){
        case 0:
            printchar(' ',74,23);
            printchar(' ',73,24);
            printchar('o',75,23);
            printchar('o',75,24);
            break;
        case 1:
            printchar(' ',75,23);
            printchar(' ',75,24);
            printchar('o',76,23);
            printchar('o',77,24);
            break;
        case 2:
            printchar(' ',76,23);
            printchar(' ',77,24);
            printchar('o',76,22);
            printchar('o',77,22);
            printchar('o',78,22);
            break;
        case 3:
            printchar(' ',76,22);
            printchar(' ',77,22);
            printchar(' ',78,22);
            printchar('o',76,21);
            printchar('o',77,20);
            break;
        case 4:
            printchar(' ',76,21);
            printchar(' ',77,20);
            printchar('o',75,21);
            printchar('o',75,20);
            break;
        case 5: 
            printchar(' ',75,21);
            printchar(' ',75,20);
            printchar('o',74,21);
            printchar('o',73,20);
            break;
        case 6:
            printchar(' ',74,21);
            printchar(' ',73,20);
            printchar('o',74,22);
            printchar('o',73,22);
            printchar('o',72,22);
            break;
        case 7:
            printchar(' ',74,22);
            printchar(' ',73,22);
            printchar(' ',72,22);
            printchar('o',74,23);
            printchar('o',73,24);
            break;
    }
}

void timerFunc(){
    for (int i = 0; i < TIMERMAX; ++i)
    {
        if(sleepTimer[i][0]>0){
            --sleepTimer[i][0];
            if(sleepTimer[i][0]==0){
                wakeupPcb(sleepTimer[i][1]);
            }
        }
    }
}

int Int21hHandeller(int choose,int arg1){
    if(choose==0){
        return _getch_();
    }else if(choose==1){
        clearScrean();
        return 0;
    }else if(choose==2){
        return _getTime_();
    }else if(choose==3){
        doFork();
    }else if(choose==4){
        doExit();
    }else if(choose==5){
        doSleep(arg1);
    }else if(choose==6){
        doWait(arg1);
    }else if(choose==101){
        return doGetSem(arg1);
    }else if(choose==102){
        return doFreeSem(arg1);
    }else if(choose==103){
        return doP(arg1);
    }else if(choose==104){
        return doV(arg1);
    }else if(choose==200){
        floppyReadSector(arg1,(unsigned char * )0x3000);
    }
    return 1;
}
void Int22hHandeller(){
    memcpy(cpuReg,&(pcbs[curPpcb].cpureg),sizeof(CPUreg));
    return;
}

