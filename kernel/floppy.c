#include "fdreg.h"
#include "libfun.h"
#include "publicLib.h"

static int reset = 1;


void SetDMA(unsigned char *buf, unsigned char cmd);

static void output_byte(unsigned char byte)//FDC 命令输出
{
	int counter;
	unsigned char status;

	for(counter = 0; counter < 10000; counter++){
		status = portIn(FD_STATUS) & (STATUS_READY | STATUS_DIR);
		if(status == STATUS_READY){
			portOut(FD_DATA, byte);
			return;
		}
	}
	reset = 1;
	print("not to send FDC\n");
}

/* 
	irq6 的中断处理函数 
	内核初始化时要打开irq6 软盘中断
	并且初始化中断门
*/
void floppy_reset_handler()
{
	if(reset){
		floppy_reset();
	}
    output_byte(FD_SPECIFY);
	output_byte(0xCF);
	output_byte(6);/* head load time = 6ms, DMA  mode*/	
}



void floppy_reset(){
	//u16 t = portIn(FD_STATUS);
	//disp_int(t,HEX);
	reset = 0;
	__asm__ ("cli");
	portOut(FD_DOR,0x1C & ~0x04);
	for(int i = 0;i < 100;i++){
		__asm__("nop");
	}	
	portOut(FD_DOR, 0x1c);
	portOut(FD_DCR,0x00);
	__asm__ ("sti");
	
}


typedef struct {
	unsigned int size, sect, head, track, strech;
	unsigned char gap, rate ,spec1;
}floppy_struct;

static floppy_struct floppy_type = \
	{2800, 18, 2, 80, 0 , 0x1B, 0x00,0xCF};
/* 1.44MB floppy */

static unsigned int current_dev = 0;

/*  
	从指定扇区号开始读磁盘，一次读取一个扇区 
	每次读取的字节数在SetDMA里设置
	DMA只能寻址到1M内存，如果buf位置大于1M
	要用位于1M以内的内存作为缓冲区，再拷贝
*/
void floppyReadSector(unsigned int sectNo, unsigned char * buf)
{
	floppy_reset();

	
	unsigned int head, track, block, sector, seek_track;

	if(buf == 0){
		return;
	}

	if(sectNo >= (floppy_type.head * floppy_type.track * floppy_type.sect)){
		return;
	}

	sector = sectNo % floppy_type.sect + 1;
	block = sectNo / floppy_type.sect;
	track = block / floppy_type.head;
	head = block % floppy_type.head;
	seek_track = track << floppy_type.strech;

	/* 软盘重新校正 */

	
	output_byte(FD_RECALIBRATE);
	output_byte(0x00);

	/* 寻找磁道 */

    output_byte(FD_SEEK);

    output_byte(0);

    output_byte(seek_track);
	
    /* 设置DMA，准备传送数据 */

    SetDMA(buf, FD_READ);


    output_byte(FD_READ); /* command */

    output_byte(0); /* driver no. */

    output_byte(track); /* track no. */

    output_byte(head); /* head */

    output_byte(sector); /* start sector */

    output_byte(2); /* sector size = 512 */

    output_byte(floppy_type.sect); /* Max sector */

    output_byte(floppy_type.gap); /* sector gap */

    output_byte(0xFF); /* sector size (0xff when n!=0 ?) */

	reset = 1;


	/* 
		DMA 方式不占用cpu，为了方便处理，增加了cpu的等待时间
		如果I/O时阻塞进程，则能增加cpu利用率(有空再写)
	*/
	for(int i = 0;i < 100000;i++){
		for(int j = 0;j < 1;j++){
			__asm__ ("nop");
		}	
	}
}


void floppyWriteSector(unsigned int sectNo, unsigned char * buf)
{
	floppy_reset();

	unsigned int head, track, block, sector, seek_track;

	if(buf == 0){
		return;
	}

	if(sectNo >= (floppy_type.head * floppy_type.track * floppy_type.sect)){
		return;
	}

	sector = sectNo % floppy_type.sect + 1;
	block = sectNo / floppy_type.sect;
	track = block / floppy_type.head;
	head = block % floppy_type.head;
	seek_track = track << floppy_type.strech;

	/* 软盘重新校正 */

	
	output_byte(FD_RECALIBRATE);
	output_byte(0x00);

	/* 寻找磁道 */

    output_byte(FD_SEEK);

    output_byte(0);

    output_byte(seek_track);
	
    /* 设置DMA，准备传送数据 */

    SetDMA(buf, FD_WRITE);


    output_byte(FD_WRITE); /* command */

    output_byte(0); /* driver no. */

    output_byte(track); /* track no. */

    output_byte(head); /* head */

    output_byte(sector); /* start sector */

    output_byte(2); /* sector size = 512 */

    output_byte(floppy_type.sect); /* Max sector */

    output_byte(floppy_type.gap); /* sector gap */

    output_byte(0xFF); /* sector size (0xff when n!=0 ?) */

	reset = 1;
	for(int i = 0;i < 100000;i++){
		for(int j = 0;j < 10;j++){
			__asm__ ("nop");
		}	
	}
}


// #define immoutb_p(val,port) \

// __asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (val)),"i" (port))



void SetDMA(unsigned char *buf, unsigned char cmd)
{

    long addr = (long)buf;

	__asm__ ("cli");

    /* mask DMA 2 */

	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (4|2)),"i" (10));

    //immoutb_p(4|2,10);

    /* output command byte. I don't know why, but everyone (minix, */

    /* sanches & canton) output this twice, first to 12 then to 11 */

    __asm__("outb %%al,$12\n\tjmp 1f\n1:\tjmp 1f\n1:\t"

    "outb %%al,$11\n\tjmp 1f\n1:\tjmp 1f\n1:"::

    "a" ((char) ((cmd == FD_READ)?DMA_READ:DMA_WRITE)));

    /* 8 low bits of addr */
    //immoutb_p(addr,4);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (addr)),"i" (4));

    addr >>= 8;

    /* bits 8-15 of addr */
    //immoutb_p(addr,4);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (addr)),"i" (4));

    addr >>= 8;

    /* bits 16-19 of addr */
    //immoutb_p(addr,0x81);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (addr)),"i" (0x81));

	/* 设置每次读取的字节数 */
    /* low 8 bits of count-1 (512-1=0x1ff) */
    //immoutb_p(0xff,5);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (0xff)),"i" (5));

    /* high 8 bits of count-1 */
    //immoutb_p(1,5);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (1)),"i" (5));

    /* activate DMA 2 */
    //immoutb_p(0|2,10);
	__asm__("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (0|2)),"i" (10));
    

	__asm__("sti");

}
