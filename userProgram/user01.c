#include "publicLib.h"

char blessing[200];
char furit_disk=0;
void _start(){
    int sem_blessing=getSem(0);
    int sem_needBlessing=getSem(1);

    int sem_furit=getSem(0);
    int sem_needFurit=getSem(1);
    if(myFork()){
        if(myFork()){
            while(1){
                P(sem_blessing);
                P(sem_furit);
                //临界区
                print(blessing);
                furit_disk=0;
                print("\n");
                //
                V(sem_needBlessing);
                V(sem_needFurit);
                mySleep(1000);
            }
        }else{
            int i=0;
            while(1){
                P(sem_needBlessing);
                //临界区
                strcpy(blessing,"blessing  ");
                strcpy(blessing+10,itoa(i,0,10));
                ++i;
                //
                V(sem_blessing);
            }
        }
    }else{
        while(1){
            P(sem_needFurit);
            //临界区
            furit_disk=1;
            //
            V(sem_furit);
        }

    }
}