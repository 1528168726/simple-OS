#include "publicLib.h"
#include "cStart.h"
#include "sync.h"

extern CPUreg cpuReg[20];
extern PCB pcbs[PCBSNUMBER];
extern int curPpcb;
SYNC_Semphere sem[SEMNUM];

void syncInitialize(){
    myMemset(sem,0,sizeof(sem));
}

int doGetSem(int val){
    if(val<0)
        return -1;
    for (int i = 0; i < SEMNUM; ++i)
    {
        if(sem[i].used==0){
            sem[i].val=val;
            sem[i].Ppcb=-1;
            sem[i].used=1;
            return i;
        }
    }
    return -1;
}

int doFreeSem(int semId){
    if(semId<0)
        return -1;
    sem[semId].used=0;
    return 1;
}

int doP(int semId){
    if(semId<0||semId>=SEMNUM)
        return -1;
    --sem[semId].val;
    if(sem[semId].val<0){
        int Ppcb=curPpcb;
        memcpy(&(pcbs[curPpcb].cpureg),cpuReg,sizeof(CPUreg));
        _pcbListDelete(&curPpcb,&curPpcb);
        _pcbListInsert(&sem[semId].Ppcb,&Ppcb);
        int22h();
    }
    return 0;
}

int doV(int semId){
    if(semId<0||semId>=SEMNUM)
        return -1;
    ++sem[semId].val;
    if(sem[semId].val<=0){
        int tmpPpcb=sem[semId].Ppcb;
        _pcbListDelete(&sem[semId].Ppcb,&sem[semId].Ppcb);
        _pcbListInsert(&curPpcb,&tmpPpcb);
    }
}