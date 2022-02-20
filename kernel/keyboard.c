#include "keyboard.h"
#include "keymap.h"
#include "libfun.h"
KB_INPUT    kb_in;
char inProcess(int key);
int code_with_E0;
int shift_l;    /* l shift state */
int shift_r;    /* r shift state */
int alt_l;      /* l alt state   */
int alt_r;      /* r left state  */
int ctrl_l;     /* l ctrl state  */
int ctrl_r;     /* l ctrl state  */
int caps_lock;  /* Caps Lock     */
int num_lock;   /* Num Lock  */
int scroll_lock;    /* Scroll Lock   */
int column;

char  get_byte_from_kbuf();

// int y=10;
// int x=50;
// int sw=0;
// void ouch(){
//     x=50;
//     ++sw;
//     if(sw%2==0) return;
//     int tmpx=x,tmpy=(y>10)?y-1:24;
//     printString("               ",&tmpx,&tmpy);
//     tmpx=x;tmpy=y;
//     printString("OUCH!haha OUCH!",&tmpx,&tmpy);
//     y=(y==24)?10:y+1;
// } 

/*======================================================================*
                            keyboard_handler
 *======================================================================*/
void keyboardHandler()
{
    char scan_code = portIn(0x60);
    // ouch();
    if (kb_in.count < KB_IN_BYTES) {
        *(kb_in.p_head) = scan_code;
        kb_in.p_head++;
        if (kb_in.p_head == kb_in.buf + KB_IN_BYTES) {
            kb_in.p_head = kb_in.buf;
        }
        kb_in.count++;
    }
}


/*======================================================================*
                           init_keyboard
*======================================================================*/
void initKeyboard()
{
    kb_in.count = 0;
    kb_in.p_head = kb_in.p_tail = kb_in.buf;

    shift_l = shift_r = 0;
    alt_l   = alt_r   = 0;
    ctrl_l  = ctrl_r  = 0;
}


/*======================================================================*
                           keyboard_read
*======================================================================*/
struct keyGet keyboardRead()
{
    char  scan_code;
    char    output[2];
    int make;   // 1: make;  0: break. 

    struct keyGet kg;
    kg.isGet=0;
    kg.c=0;
    int key = 0;
             
    int*    keyrow; /* 指向 keymap[] 的某一行 */

    if(kb_in.count > 0){
        code_with_E0 = 0;

        scan_code = get_byte_from_kbuf();

        /* 下面开始解析扫描码 */
        if (scan_code == 0xE1) {
            int i;
            char pausebrk_scode[] = {0xE1, 0x1D, 0x45,
                           0xE1, 0x9D, 0xC5};
            int is_pausebreak = 1;
            for(i=1;i<6;i++){
                if (get_byte_from_kbuf() != pausebrk_scode[i]) {
                    is_pausebreak = 0;
                    break;
                }
            }
            if (is_pausebreak) {
                key = PAUSEBREAK;
            }
        }
        else if (scan_code == 0xE0) {
            scan_code = get_byte_from_kbuf();

            /* PrintScreen 被按下 */
            if (scan_code == 0x2A) {
                if (get_byte_from_kbuf() == 0xE0) {
                    if (get_byte_from_kbuf() == 0x37) {
                        key = PRINTSCREEN;
                        make = 1;
                    }
                }
            }
            /* PrintScreen 被释放 */
            if (scan_code == 0xB7) {
                if (get_byte_from_kbuf() == 0xE0) {
                    if (get_byte_from_kbuf() == 0xAA) {
                        key = PRINTSCREEN;
                        make = 0;
                    }
                }
            }
            /* 不是PrintScreen, 此时scan_code为0xE0紧跟的那个值. */
            if (key == 0) {
                code_with_E0 = 1;
            }
        }
        if ((key != PAUSEBREAK) && (key != PRINTSCREEN)) {
            /* 首先判断Make Code 还是 Break Code */
            make = (scan_code & FLAG_BREAK ? 0 : 1);

            /* 先定位到 keymap 中的行 */
            keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
            
            column = 0;
            if (shift_l || shift_r) {
                column = 1;
            }
            if (code_with_E0) {
                column = 2; 
                code_with_E0 = 0;
            }
            
            key = keyrow[column];
            
            switch(key) {
            case SHIFT_L:
                shift_l = make;
                break;
            case SHIFT_R:
                shift_r = make;
                break;
            case CTRL_L:
                ctrl_l = make;
                break;
            case CTRL_R:
                ctrl_r = make;
                break;
            case ALT_L:
                alt_l = make;
                break;
            case ALT_R:
                alt_l = make;
                break;
            default:
                break;
            }

            if (make) { /* 忽略 Break Code */
                key |= shift_l  ? FLAG_SHIFT_L  : 0;
                key |= shift_r  ? FLAG_SHIFT_R  : 0;
                key |= ctrl_l   ? FLAG_CTRL_L   : 0;
                key |= ctrl_r   ? FLAG_CTRL_R   : 0;
                key |= alt_l    ? FLAG_ALT_L    : 0;
                key |= alt_r    ? FLAG_ALT_R    : 0;
                
                // inProcess(key);
                kg.isGet=1;
                kg.c=key;
                return kg;
            }
        }
    }
    return kg;
}
 
/*======================================================================*
                get_byte_from_kbuf
 *======================================================================*/
char get_byte_from_kbuf()       /* 从键盘缓冲区中读取下一个字节 */
{
        char scan_code;

        while (kb_in.count <= 0) {}   /* 等待下一个字节到来 */

        disableInt();
        scan_code = *(kb_in.p_tail);
        kb_in.p_tail++;
        if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
                kb_in.p_tail = kb_in.buf;
        }
        kb_in.count--;
        enableInt();

        return scan_code;
}

