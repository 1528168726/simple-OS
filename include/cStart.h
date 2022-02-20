#ifndef _CSTART_H_
#define _CSTART_H_
#define startInterruptVector 0x10
#define NOFATHER -1
#define PCBSNUMBER 20
#define TIMERMAX 20
#define IDOFFSET 0x1000
#define STACKPOSITION 0x210000 //栈池从0x200000到0x300000
typedef struct CPUreg{
    int ss;
    int esp;
    int retAddr;
    int eip;
    int cs;
    int eflags;
    int gs;
    int fs;
    int es;
    int ds;
    int edi;
    int esi;
    int ebp;
    int kernel_esp;
    int ebx;
    int edx;
    int ecx;
    int eax;
}CPUreg;

typedef struct PCB
{
    CPUreg cpureg;
    int id;
    int stackBase;
    int Pnext;
    int Ppre;
    int used;
    int isBlocked;
    int fatherPpcb;
    char name[64];
}PCB;
//外部函数
extern void initKeyboard();
extern void keyboardHandler();
extern void _keyboardHandler();
extern void _ClockHandler();
extern void _Int20h();
extern void _Int21h();
extern void _Int22h();
//内部函数
void setTimebreak();
void Init8259A();
void setIDT();
void ClockHandler();
int readyPcbAdd(CPUreg* cpureg);
int readyPcbNormalAdd(int cs,int eip,int ds);
void blockPcb(int Ppcb);
void wakeupPcb(int Ppcb);
int doFork();
void doExit();
void doSleep();
int _pcbListDelete(int*Ppcb,int* toDeletePpcb);
int _pcbListInsert(int*Ppcb,int* toInsertPpcb);
int getFreePcb();
void setPcb(int Ppcb,const CPUreg* cpureg,char*name,int stackBase,int fatherPpcb);
int getNewStack();
void freeStack(int stackBase);
void hotWheels(int clock);
void timerFunc();
int Int21hHandeller(int choose,int arg1);
void Int22hHandeller();

typedef struct GATE
{
    unsigned short offset_low; 
    unsigned short selector;   
    unsigned short attribute;  //属性 
    unsigned short offset_high;    
}GATE;

#define IGATE 0x8E00
#define CGate 0x8C00
#define TGate 0x8F00

#endif