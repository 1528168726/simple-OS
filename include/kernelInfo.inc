stackTopAddr equ 0x2fff0
IDTAddr equ 0x90000
userProgramAddr equ 0x70000
userProgramBaseSpace equ 0x08000   ;每个用户程序最大32k
cpuRegTop equ 4*18
cpuRegPushNum equ 4*12