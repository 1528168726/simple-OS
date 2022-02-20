[bits 16]
    jmp realStart

%include "protectMode.inc"
%include "loadInfo.inc"
%include  "fat12hdr.inc"
;GDT---------------------------------------------------------------------------------
GdtDefault:             Descriptor  0,          0,          0;空描述符
GdtAllCode:             Descriptor  0,          0xfffff,    DA_32 | DA_CR | DA_LIMIT_4K
GdtAllData:             Descriptor  0,          0xfffff,    DA_32 | DA_DRW | DA_LIMIT_4K
GdtVideo:               Descriptor  0xB8000,    0x48000,    DA_DPL3 | DA_DRW 
;====================================================================================
GdtSize equ $-GdtDefault
GdtInfo dw GdtSize-1
        dd loaderAddr+GdtDefault

;GDT Selector-------------------------------------------------------------------------
SelectorAllCode      equ     GdtAllCode-GdtDefault
SelectorAllData      equ     GdtAllData-GdtDefault
SelectorVideo           equ     GdtVideo-GdtDefault+ SA_RPL3
;=====================================================================================

;读取文件信息
kernelFileName      db  "KERNEL  BIN", 0
userProgramBase equ 0x7000
userProgramOff equ 0
userProgramBaseSpace equ 0x0800   ;每个用户程序最大32k
user01FileName      db  "USER01  ELF", 0
user02FileName      db  "USER02  ELF", 0
user03FileName      db  "USER03  ELF", 0
;

realStart:
    mov ax,cs
    mov es,ax
    mov ds,ax
    mov ss,ax
    mov sp,loaderTop

    call readElfFile ;从软盘读内核等elf文件
    ;进入保护模式
    ; 加载 GDTR
    lgdt    [GdtInfo]

    ; 关中断
    cli

    ; 打开地址线A20
    in  al, 92h
    or  al, 00000010b
    out 92h, al

    ; 准备切换到保护模式
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    call dword SelectorAllCode:(loaderAddr+protectStart)

;读磁盘变量    
wRootDirSizeForLoop dw  RootDirSectors  ; Root Directory 占用的扇区数
wSectorNo       dw  0       ; 要读取的扇区号
bOdd            db  0       ; 奇数还是偶数
OffsetOfFile dw 0 ;保存文件的偏移地址
BaseOfFile  dw 0  ;保存文件的段地址
FileNameAddr dw 0; 
;读磁盘函数-------------------------------------------------------------
readDisk:
    xor ah, ah  ; ┓
    xor dl, dl  ; ┣ 软驱复位
    int 13h ; ┛
    mov word [wSectorNo], SectorNoOfRootDirectory   
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmp word [wRootDirSizeForLoop], 0   ; ┓
    jz  LABEL_NO_LOADERBIN      ; ┣ 判断根目录区是不是已经读完
    dec word [wRootDirSizeForLoop]  ; ┛ 如果读完表示没有找到 LOADER.BIN
    mov ax, word[BaseOfFile]
    mov es, ax          ; es <- BaseOfLoader
    mov bx, word[OffsetOfFile]  ; bx <- OffsetOfLoader  于是, es:bx = BaseOfLoader:OffsetOfLoader
    mov ax, [wSectorNo] ; ax <- Root Directory 中的某 Sector 号
    mov cl, 1
    call    ReadSector

    mov si, word[FileNameAddr]  ; ds:si -> "LOADER  BIN"
    mov di, word[OffsetOfFile]  ; es:di -> BaseOfLoader:0100 = BaseOfLoader*10h+100
    cld
    mov dx, 10h
LABEL_SEARCH_FOR_LOADERBIN:
    cmp dx, 0                   ; ┓循环次数控制,
    jz  LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR  ; ┣如果已经读完了一个 Sector,
    dec dx                  ; ┛就跳到下一个 Sector
    mov cx, 11
LABEL_CMP_FILENAME:
    cmp cx, 0
    jz  LABEL_FILENAME_FOUND    ; 如果比较了 11 个字符都相等, 表示找到
dec cx
    lodsb               ; ds:si -> al
    cmp al, byte [es:di]
    jz  LABEL_GO_ON
    jmp LABEL_DIFFERENT     ; 只要发现不一样的字符就表明本 DirectoryEntry 不是
; 我们要找的 LOADER.BIN
LABEL_GO_ON:
    inc di
    jmp LABEL_CMP_FILENAME  ;   继续循环

LABEL_DIFFERENT:
    and di, 0FFE0h      ; else ┓    di &= E0 为了让它指向本条目开头
    add di, 20h         ;      ┃
    mov si, word[FileNameAddr]  ;      ┣ di += 20h  下一个目录条目
    jmp LABEL_SEARCH_FOR_LOADERBIN;    ┛

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
    add word [wSectorNo], 1
    jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
    jmp $           ; 没有找到 LOADER.BIN, 死循环在这里

LABEL_FILENAME_FOUND:           ; 找到 LOADER.BIN 后便来到这里继续
    mov ax, RootDirSectors
    and di, 0FFE0h      ; di -> 当前条目的开始
    add di, 01Ah        ; di -> 首 Sector
    mov cx, word [es:di]
    push    cx          ; 保存此 Sector 在 FAT 中的序号
    add cx, ax
    add cx, DeltaSectorNo   ; 这句完成时 cl 里面变成 LOADER.BIN 的起始扇区号 (从 0 开始数的序号)
    mov ax, word[BaseOfFile]
    mov es, ax          ; es <- BaseOfLoader
    mov bx, word[OffsetOfFile]  ; bx <- OffsetOfLoader  于是, es:bx = BaseOfLoader:OffsetOfLoader = BaseOfLoader * 10h + OffsetOfLoader
    mov ax, cx          ; ax <- Sector 号

LABEL_GOON_LOADING_FILE:
    push    ax          ; ┓
    push    bx          ; ┃
    mov ah, 0Eh         ; ┃ 每读一个扇区就在 "Booting  " 后面打一个点, 形成这样的效果:
    mov al, '.'         ; ┃
    mov bl, 0Fh         ; ┃ Booting ......
    int 10h         ; ┃
    pop bx          ; ┃
    pop ax          ; ┛

    mov cl, 1
    call    ReadSector
    pop ax          ; 取出此 Sector 在 FAT 中的序号
    call    GetFATEntry
    cmp ax, 0FFFh
    jz  LABEL_FILE_LOADED
    push    ax          ; 保存 Sector 在 FAT 中的序号
    mov dx, RootDirSectors
    add ax, dx
    add ax, DeltaSectorNo
    add bx, [BPB_BytsPerSec]
    jmp LABEL_GOON_LOADING_FILE
LABEL_FILE_LOADED:
    ret
    

ReadSector:
    ; -----------------------------------------------------------------------
    ; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
    ; -----------------------------------------------------------------------
    ; 设扇区号为 x
    ;                           ┌ 柱面号 = y >> 1
    ;       x           ┌ 商 y ┤
    ; -------------- => ┤      └ 磁头号 = y & 1
    ;  每磁道扇区数     │
    ;                   └ 余 z => 起始扇区号 = z + 1
    push    bp
    mov bp, sp
    sub esp, 2          ; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

    mov byte [bp-2], cl
    push    bx          ; 保存 bx
    mov bl, [BPB_SecPerTrk] ; bl: 除数
    div bl          ; y 在 al 中, z 在 ah 中
    inc ah          ; z ++
    mov cl, ah          ; cl <- 起始扇区号
    mov dh, al          ; dh <- y
    shr al, 1           ; y >> 1 (其实是 y/BPB_NumHeads, 这里BPB_NumHeads=2)
    mov ch, al          ; ch <- 柱面号
    and dh, 1           ; dh & 1 = 磁头号
    pop bx          ; 恢复 bx
    ; 至此, "柱面号, 起始扇区, 磁头号" 全部得到 ^^^^^^^^^^^^^^^^^^^^^^^^
    mov dl, [BS_DrvNum]     ; 驱动器号 (0 表示 A 盘)
.GoOnReading:
    mov ah, 2           ; 读
    mov al, byte [bp-2]     ; 读 al 个扇区
    int 13h
    jc  .GoOnReading        ; 如果读取错误 CF 会被置为 1, 这时就不停地读, 直到正确为止

    add esp, 2
    pop bp

    ret

;----------------------------------------------------------------------------
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;   找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;   需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
    push    es
    push    bx
    push    ax
    mov ax, word[BaseOfFile]    ; ┓
    sub ax, 0100h       ; ┣ 在 BaseOfKernelFile 后面留出 4K 空间用于存放 FAT
    mov es, ax          ; ┛
    pop ax
    mov byte [bOdd], 0
    mov bx, 3
    mul bx          ; dx:ax = ax * 3
    mov bx, 2
    div bx          ; dx:ax / 2  ==>  ax <- 商, dx <- 余数
    cmp dx, 0
    jz  LABEL_EVEN
    mov byte [bOdd], 1
LABEL_EVEN:;偶数
    xor dx, dx          ; 现在 ax 中是 FATEntry 在 FAT 中的偏移量. 下面来计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
    mov bx, [BPB_BytsPerSec]
    div bx          ; dx:ax / BPB_BytsPerSec  ==>   ax <- 商   (FATEntry 所在的扇区相对于 FAT 来说的扇区号)
                    ;               dx <- 余数 (FATEntry 在扇区内的偏移)。
    push    dx
    mov bx, 0           ; bx <- 0   于是, es:bx = (BaseOfKernelFile - 100):00 = (BaseOfKernelFile - 100) * 10h
    add ax, SectorNoOfFAT1  ; 此句执行之后的 ax 就是 FATEntry 所在的扇区号
    mov cl, 2
    call    ReadSector      ; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界发生错误, 因为一个 FATEntry 可能跨越两个扇区
    pop dx
    add bx, dx
    mov ax, [es:bx]
    cmp byte [bOdd], 1
    jnz LABEL_EVEN_2
    shr ax, 4
LABEL_EVEN_2:
    and ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

    pop bx
    pop es
    ret
;----------------------------------------------------------------------------

readElfFile:
    mov word[BaseOfFile],kernelSeg
    mov word[OffsetOfFile],kernelOffset
    mov word[FileNameAddr],kernelFileName
    call readDisk
    mov word[BaseOfFile],userProgramBase+0*userProgramBaseSpace
    mov word[OffsetOfFile],userProgramOff
    mov word[FileNameAddr],user01FileName
    call readDisk
    mov word[BaseOfFile],userProgramBase+1*userProgramBaseSpace
    mov word[OffsetOfFile],userProgramOff
    mov word[FileNameAddr],user02FileName
    call readDisk
    mov word[BaseOfFile],userProgramBase+2*userProgramBaseSpace
    mov word[OffsetOfFile],userProgramOff
    mov word[FileNameAddr],user03FileName
    call readDisk
    ret


;-----------------------------------------------------------------------------------


[bits 32]
protectStart:
    mov ax,SelectorAllData      ;设置段寄存器
    mov ds,ax
    mov ss,ax
    mov es,ax
    mov fs,ax
    mov esp,loaderAddr+loaderTop
    mov ax,SelectorVideo
    mov gs,ax

    call loadKernel ;加载内核

    ;跳转到内核中
    mov ax,SelectorAllCode
    push ax
    mov eax,dword[KernelEntry]
    push eax
    retf 


loadKernel:      ;已测试
    mov eax,dword[kernelAddr+0x18]
    mov dword[KernelEntry],eax          ;读取内核入口
    mov esi,dword[kernelAddr+0x1c]           ;读取program header table 的在文件的偏移
    add esi,kernelAddr
    mov cx,word[kernelAddr+0x2c]       ;读取phnum，Program header table 的数目
    movzx ecx,cx
.loop1:
    mov eax,dword[esi]
    cmp eax,0
    jz  .skip
    mov eax,dword[esi+0x10]     ;file size
    push eax
    mov eax,dword[esi+0x04]     ;file offsize
    add eax,kernelAddr          
    push eax
    mov eax,dword[esi+0x08]     ;dest
    push eax
    call memcpy
    add esp,4*3
.skip:
    add esi,0x20
    loop .loop1
    ret

;memcpy-----------------------------------------------------------
memcpy:     ;void *memcpy(void *destin, void *source, unsigned n)
    push ebp
    mov ebp,esp
    push ecx
    push esi
    push edi

    mov edi,dword[ebp+8]    ;void*destin
    mov esi,dword[ebp+12]   ;void*source
    mov ecx,dword[ebp+16]   ;void*n

.1:
    cmp ecx,0
    jz .1out
    mov al,byte[esi]
    mov byte[edi],al
    inc esi
    inc edi
    dec ecx
    jmp .1
.1out:

    mov eax,dword[ebp+8]

    pop edi
    pop esi
    pop ecx
    pop ebp
    ret
;==================================================================

dataDef:
    KernelEntry dd 0

