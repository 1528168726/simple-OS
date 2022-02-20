#变量：
NASMBootFlag = -I boot/include/ -f bin
NASMKernelFlag = -I include/ -felf32
CFlag =  -fno-stack-protector -m32 -I include/ -c -O0
LDFlag = -s -Ttext 0x30400 -m elf_i386

kernelObjects = kernel/kernel.o kernel/alibfun.o kernel/cStart.o \
				kernel/keyboard.o kernel/clibfun.o kernel/publicLib.o \
				kernel/aPublicLib.o kernel/tty.o kernel/sync.o \
				kernel/floppy.o 
bootFile = boot/boot.bin boot/loader.bin
mntDir = /mnt/floppy
userProgram = userProgram/user01.elf userProgram/user02.elf userProgram/user03.elf
kernelFile = kernel.bin
publicLib = kernel/publicLib.o kernel/aPublicLib.o

.PHONY : makeImage 
buildImage : $(bootFile) $(kernelFile) $(userProgram)
	cp default.img pxjOS.img
	dd if=boot/boot.bin of=pxjOS.img bs=512 count=1 conv=notrunc
	sudo mount -oloop pxjOS.img $(mntDir)
	sudo cp -f $(kernelFile) $(mntDir)
	sudo cp -f $(bootFile) $(mntDir)
	sudo cp -f $(userProgram) $(mntDir)
	sudo cp -f tmp111.txt $(mntDir)
	sudo umount /mnt/floppy

debug_boot: $(bootFile)
	./createFloppyDisk $(bootFile) -o pxjOS.img

kernel.bin: $(kernelObjects)
	ld $(LDFlag) $(kernelObjects) -o $(kernelFile)


kernel/cStart.o: kernel/cStart.c include/*
	gcc $(CFlag) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c include/*
	gcc $(CFlag) -o $@ $<

kernel/clibfun.o: kernel/clibfun.c include/*
	gcc $(CFlag) -o $@ $<

kernel/tty.o: kernel/tty.c kernel/testFun.h include/* 
	gcc $(CFlag) -o $@ $<

kernel/publicLib.o: kernel/publicLib.c include/*
	gcc $(CFlag) -o $@ $<

kernel/sync.o: kernel/sync.c include/*
	gcc $(CFlag) -o $@ $<

kernel/floppy.o: kernel/floppy.c include/*
	gcc $(CFlag) -o $@ $<

kernel/kernel.o: kernel/kernel.asm include/*
	nasm $(NASMKernelFlag) -o $@ $<

kernel/alibfun.o: kernel/alibfun.asm include/*
	nasm $(NASMKernelFlag) -o $@ $<

kernel/aPublicLib.o:kernel/aPublicLib.asm include/*
	nasm $(NASMKernelFlag) -o $@ $<

boot/boot.bin: boot/boot.asm boot/include/*
	nasm $(NASMBootFlag) -o $@ $<

boot/loader.bin: boot/loader.asm boot/include/*
	nasm $(NASMBootFlag) -o $@ $<


userProgram/user01.o: userProgram/user01.c include/*
	gcc $(CFlag) -o $@ $<

userProgram/user02.o: userProgram/user02.c include/*
	gcc $(CFlag) -o $@ $<

userProgram/user03.o: userProgram/user03.c include/*
	gcc $(CFlag) -o $@ $<

userProgram/user01.elf: userProgram/user01.o $(publicLib)
	ld  -s -Ttext 0x300400 -m elf_i386 $^ -o $@

userProgram/user02.elf: userProgram/user02.o $(publicLib)
	ld  -s -Ttext 0x310400 -m elf_i386 $^ -o $@

userProgram/user03.elf: userProgram/user03.o $(publicLib)
	ld  -s -Ttext 0x320400 -m elf_i386 $^ -o $@
	
# clean :
# 	rm -f edit $(objects)