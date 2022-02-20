#ifndef _SYNC_H_
#define _SYNC_H_

#define SEMNUM 20

typedef struct SYNC_Semphere
{
    int val;
    int Ppcb;
    int used;
}SYNC_Semphere;

void syncInitialize();
int doGetSem(int val);
int doFreeSem(int semId);
int doP(int semId);
int doV(int semId);

#endif