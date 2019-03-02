/* --------------------------------------------------------------------------
 * For Terminal I/O 
 * 2012 - 4 - 19 
 * By Casey
 * --------------------------------------------------------------------------*/

#ifndef __TERMIO_H__
#define __TERMIO_H__

#define RGB_BLACK           0x00
#define RGB_BLUE            0x01
#define RGB_GREEN           0x02
#define RGB_DARK_GREEN      0x03
#define RGB_RED             0x04
#define RGB_DARK_RED        0x05
#define RGB_PURP            0x06
#define RGB_WHITE           0x07

#define MAX_CONSOLES        4
#define MAX_CHAR_BUF_LEN    1025

/* key board in buffer que */
typedef struct tagCHAR_BUF{
    short   head,tail,len;
    unsigned char   buf[MAX_CHAR_BUF_LEN];
    unsigned char   locked ;
}CHAR_BUF;

typedef struct tagCOOKED_BUF{
    char buf[MAX_CHAR_BUF_LEN];
    char locked;
}COOKED;

//CRT CONTROLLER REGS 
#define CRT_ADDR             0x3D4
#define CRT_DATA             0x3D5
#define CRT_START_H          0x0C
#define CRT_START_L          0x0D
#define CRT_CURPOS_H         0x0E
#define CRT_CURPOS_L         0x0F
#define VIDEO_MEM_BASE       0xB8000


/* console */
typedef struct {
    long   cur_start_addr;     //console show start addr
    long   org_addr;           //org addr 
    long   max_addr;           //current console max mem size
    long   curpos;             //current cursor pos
    char   rgb;                //console rgb 
}CONSOLE;

/* common buffer */

/* current console */
typedef struct {
    CONSOLE console [MAX_CONSOLES];
    unsigned char cur_con_nr;
}X;


#define KB_BUF_LEN  (16)
typedef struct tagKBD_QUE{
    char            head,tail,len;
    char            buf[KB_BUF_LEN];
    unsigned char   locked;
}KBD_QUE;

#endif
