%include "kernelInfo.inc"
[bits 32]
[section .text]
extern memcpy
global loadProgramElf
global portOut
global portIn
global printchar
global getIDTAddr
global disableInt
global enableInt
global KerneljmpFun
global FunRetKernel


;loadProgramElf(int i) i从1开始
loadProgramElf:
    pushad
    mov ecx,dword[esp+36]
    dec ecx
    imul ecx,userProgramBaseSpace
    mov ebx,userProgramAddr
    add ebx,ecx     ;ebx 为elf位置

    mov eax,dword[ebx+0x18] ;返回内核入口
    push eax
    mov esi,dword[ebx+0x1c]           ;读取program header table 的在文件的偏移
    add esi,ebx
    mov cx,word[ebx+0x2c]       ;读取phnum，Program header table 的数目
    movzx ecx,cx
.loop1:
    mov eax,dword[esi]
    cmp eax,0
    jz  .skip
    mov eax,dword[esi+0x10]     ;file size
    push eax
    mov eax,dword[esi+0x04]     ;file offsize
    add eax,ebx          
    push eax
    mov eax,dword[esi+0x08]     ;dest
    push eax
    call memcpy
    add esp,4*3
.skip:
    add esi,0x20
    loop .loop1
    pop eax
    popad
    mov eax,dword[ss:esp-36]
    ret




;向端口输出---------------------------------------------------------
;void portOut(short port, unsigned char num )
portOut:
    mov edx,dword[esp+4]
    mov eax,dword[esp+8]
    out dx,al
    nop
    nop
    nop
    nop 
    nop
    nop
    nop
    nop 
    ret
;===================================================================


;从端口输入----------------------------------------------------------
;unsigned char portIn(short port)
portIn:
    xor eax,eax
    mov edx,dword[esp+4]
    in  al,dx
    nop
    nop
    nop
    nop 
    nop
    nop
    nop
    nop 
    ret
;===================================================================


;void printchar(char a,int x, int y)--------------------------------
printchar:
    push ebp
    push ebx
    xor eax,eax                 ; 计算显存地址
    mov eax,dword[esp+12+8]
    mov bx,80
    mul bx
    add eax,dword[esp+8+8]
    mov bx,2
    mul bx
    mov bp,ax
    mov eax,dword[esp+4+8]           ;  AL = 显示字符值（默认值为20h=空格符）
    mov ah,0Fh              ;  0000：黑底、1111：亮白字（默认值为07h）
    mov word[gs:bp],ax          ;  NASM汇编，显示字符的ASCII码值
;   mov word es:[bp],ax         ;  TASM/MASM汇编，显示字?的ASCII码值
    pop ebx
    pop ebp
    ret
;===================================================================


;int getIDTAddr()--------------------------------------------------
getIDTAddr:
    mov eax,IDTAddr
    ret
;===================================================================


;void disableInt()---------------------------------------------------
disableInt:
    cli
    ret
;===================================================================


;void enableInt()---------------------------------------------------
enableInt:
    sti
    ret
;===================================================================

;跳转到用户程序
;void KerneljmpFun(unsigned int fun);---------------------------------------------------
KerneljmpFun:
    pushad
    mov eax,[esp+36]
    push    ds      ;  |
    push    es      ;  | 保存原寄存器值
    push    fs      ;  |
    push    gs      ; /
    mov dword[leftEsp],esp
    mov word[leftEsp+4],ss
    jmp eax

FunRetKernel:
    mov ss,word[leftEsp+4]
    mov esp,dword[leftEsp]
    pop gs
    pop fs
    pop es
    pop ds
    popad
    ret
leftEsp:
    dd 0
    dw 0
;===================================================================
