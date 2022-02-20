%include "kernelInfo.inc"
[bits 32]
[section .text]
extern cStart
extern Init8259A
extern setIDT
extern ClockHandler
extern keyboardHandler
extern Int21hHandeller
extern Int22hHandeller
extern doExit
extern putch
extern memcpy
extern cpuReg
global _start
global _ClockHandler
global _keyboardHandler
global _Int20h
global _Int21h
global _Int22h
global _save
global _restart

_start:
    mov esp,stackTopAddr ;设置栈顶顶部
    call setIDT
    lidt [IdtPtr]
    call Init8259A
    sti 
    call cStart
    jmp $


_ClockHandler:
    call _save
    call ClockHandler
    mov al,0x20
    out 0x20,al
    jmp _restart

_keyboardHandler:
    call _save
    call keyboardHandler
    mov al,0x20
    out 0x20,al
    jmp _restart

;20h号中断，返回内核；
_Int20h:
    cli
    jmp doExit

_Int21h:
    cli
    call _save
;//-----------
    mov eax,dword[ss:esp+8]
    push eax
    mov eax,dword[ss:esp+8]
    push eax
    call Int21hHandeller
    add esp,8
    mov dword[cpuReg+cpuRegTop*1-4],eax ;设置返回值
    call _restart


_Int22h:
    cli
    call Int22hHandeller
    call _restart

_save:
    cli
    mov word[cpuReg+0],ss
    mov dword[cpuReg+4],esp ;stack:...eflags,cs,eip,retAddr,
    mov esp,cpuReg+cpuRegTop
    pushad
    push ds
    push es
    push fs
    push gs
    mov esp,dword[cpuReg+4]
    add dword[cpuReg+4],4*4
    mov eax,dword[ss:esp+4*0]
    mov dword[cpuReg+4*2],eax ;retaddr
    mov eax,dword[ss:esp+4*1]
    mov dword[cpuReg+4*3],eax ;eip
    mov eax,dword[ss:esp+4*2]
    mov dword[cpuReg+4*4],eax ;cs
    mov eax,dword[ss:esp+4*3]
    mov dword[cpuReg+4*5],eax ;eflags
    mov eax,[cpuReg+4*2]
    add esp,4*4
    jmp eax

_restart:
    mov ss,word[cpuReg+0]
    mov esp,cpuReg+cpuRegTop-cpuRegPushNum
    pop gs
    pop fs
    pop es
    pop ds
    popad
    mov esp,dword[cpuReg+4]
    push dword[cpuReg+4*5]
    push dword[cpuReg+4*4]
    push dword[cpuReg+4*3]
    sti
    iretd

IdtPtr: dw 2039
        dd IDTAddr