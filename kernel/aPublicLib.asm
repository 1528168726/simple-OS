global _printchar_
; global memcpy
global myFork
global int20h
global int21h
global int22h
global int23h
;void _printchar_(char a,int x, int y)--------------------------------
_printchar_:
    push ebp
    push ebx
    xor eax,eax                 ; 计算显存地址
    mov eax,dword[ss:esp+12+8]
    mov bx,80
    mul bx
    add eax,dword[ss:esp+8+8]
    mov bx,2
    mul bx
    mov bp,ax
    mov eax,dword[ss:esp+4+8]           ;  AL = 显示字符值（默认值为20h=空格符）
    mov ah,0Fh              ;  0000：黑底、1111：亮白字（默认值为07h）
    mov word[gs:bp],ax          ;  NASM汇编，显示字符的ASCII码值
;   mov word es:[bp],ax         ;  TASM/MASM汇编，显示字?的ASCII码值
    pop ebx
    pop ebp
    ret
;===================================================================

; ;memcpy-----------------------------------------------------------
; memcpy:     ;void *memcpy(void *destin, void *source, unsigned n)
;     push ebp
;     mov ebp,esp
;     push ecx
;     push esi
;     push edi

;     mov edi,dword[ebp+8]    ;void*destin
;     mov esi,dword[ebp+12]   ;void*source
;     mov ecx,dword[ebp+16]   ;void*n

;     mov eax,-1
;     cmp edi,0x01000000
;     jg .1out

; .1:
;     cmp ecx,0
;     jz .1out
;     mov al,byte[esi]
;     mov byte[edi],al
;     inc esi
;     inc edi
;     dec ecx
;     jmp .1
;     mov eax,dword[ebp+8]
; .1out:
;     pop edi
;     pop esi
;     pop ecx
;     pop ebp
;     ret

; ;==================================================================


myFork:
    push 3
    push 3
    push 3
    int 21h
    add esp,12
    ret

int20h:
    int 20h
    ret

int21h:
    int 21h
    ret
int22h:
    int 22h
    ret
int23h:
    int 23h
    ret

