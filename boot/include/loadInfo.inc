;loader 加载到内存位置相关信息
loaderSeg equ 0x9500
loaderTop equ 0x9f000-loaderSeg*16
loaderOffset equ 0
loaderAddr equ loaderSeg*16+loaderOffset

;kernel 读取到内存位置相关信息
kernelSeg equ 0x5000
kernelOffset equ 0
kernelAddr  equ kernelSeg*16+kernelOffset 

;kernel 

PageDirPos  equ 0x40000  ;页目录位置：第4M处
PageTblPos  equ 0x401000  ;页表位置：第4M+4k处