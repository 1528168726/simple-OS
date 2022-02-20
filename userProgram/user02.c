#include "publicLib.h"

int money=10;
void _start(){
    int id=myFork();
    if(id){
        for (int i = 0; i < 2000; ++i)
        {
            ++money;
            for (int j = 0; j < 10000; ++j)
            {
                ++money;
            }
            money-=1000;
        }
        myWait(id);
        if(money>=0)
            print(itoa(money,0,10));
        else{
            print("-");
            print(itoa(-money,0,10));
        }
        print("\n");
    }
    else{
        for (int i = 0; i < 2000; ++i)
        {
            --money;
            for (int j = 0; j < 10000; ++j)
            {
                --money;
            }
            money+=1000;
        }
    }
    myExit();
}