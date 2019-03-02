#include <sys/types.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/termio.h>
#include <casey/mm.h>

/* kernel can has the basical Terminal I/O system 
 * which has printk (),kputs (),etc ...,so the kernel 
 * can be ok without terminal task (may it happened) 
 */

static X *x = (X*)(TERM_ROCK);

static CONSOLE *console = NULL;
static unsigned char cur_con_nr  = 0;

void setcurpos ( __u32 curpos ) 
{
    wrport (CRT_ADDR,CRT_CURPOS_H);
    wrport (CRT_DATA,(curpos>>9)&0xFF );
    wrport (CRT_ADDR,CRT_CURPOS_L);
    wrport (CRT_DATA,(curpos>>1)&0xFF );
}

void init_console ( void )
{
    int i = 0 ;
    unsigned long start_addr = 0;

    console = x->console;
    cur_con_nr = x->cur_con_nr;

    for ( i = 0 ; i < MAX_CONSOLES ; i ++ ) 
    {
        console[i].cur_start_addr   =
        console[i].curpos           =
        console[i].org_addr         = start_addr ;
        console[i].max_addr         = 8000;
        console[i].rgb              = 0x0F;
        start_addr += 8000;         /* each console has 2 screen */
    }
}

CONSOLE* get_curcon ( void ) 
{

}

CONSOLE* set_curcon ( unsigned char cur_con ) 
{
    x->cur_con_nr = cur_con ;
    return ( &console[cur_con] );
}

static 
void set_start_addr ( unsigned long addr ) 
{
    wrport (CRT_ADDR,CRT_START_H);
    wrport (CRT_DATA,((addr)>>9) & 0xFF );
    wrport (CRT_ADDR,CRT_START_L);
    wrport (CRT_DATA,((addr)>>1) & 0xFF );
}

static
void set_cur_con ( CONSOLE *pcon ) 
{
    setcurpos (pcon->curpos);
    set_start_addr (pcon->cur_start_addr);
}

inline unsigned char MAKERGB ( bool bkhl,
        unsigned char bk,bool fghl,unsigned char fg ) 
{
    return ( (bkhl<<7)|(bk<<4)|(fghl<<3)|(fg) );
}

/* scroll lines lines up*/
static 
void scroll_down (unsigned char lines)
{
    long start = console[cur_con_nr].cur_start_addr;
    //unsigned long max = console[cur_con_nr].max_addr;
    //if (start = lines * 160 > max ) return;

    start += lines * 160 ;
    console[cur_con_nr].cur_start_addr = start;
    set_start_addr (start);
}

/* scroll lines lines down */
static 
void scroll_up (unsigned char lines)
{
    long start = console[cur_con_nr].cur_start_addr;
    //unsigned long max = console[cur_con_nr].max_addr;

    if ( start - lines * 160 < 0 ) return;

    start -= lines * 160 ;
    console[cur_con_nr].cur_start_addr = start;
    set_start_addr (start);
}

void set_pos ( long pos ) 
{
    if ( (pos / 160) * 160 - console[cur_con_nr].cur_start_addr >= (4000) ) 
        scroll_down (1);

    console[cur_con_nr].curpos = pos;
    setcurpos (pos);
}

int  kputch ( const char ch ) 
{
    long pos = console[cur_con_nr].curpos;
    __byte *vbase = (__byte*)(VIDEO_MEM_BASE+pos),i = 0;
    unsigned char rgb = console[cur_con_nr].rgb;
    //unsigned long max = console[cur_con_nr].max_addr;

    if (pos + 2 >= (0x8000))
    {
        panic ( "pos 0x%0X touch the bottom line !",pos );
    }

    switch ( ch ) 
    {
        case '\n':
            pos = (pos/160+1) * 160 ;   /* next 160 */
            break;
        case '\t':
            for ( i = 0 ; i < 4 ; i ++ )
            {
                *vbase ++ = ' ';
                *vbase ++ = rgb;
            }
            pos += 2 * 4;
            break;
        case '\b':
            if ( pos - 2 < 0 ) return (-1);
            pos -= 2;
            break;
        default:
            *vbase ++ = ch;
            *vbase ++ = rgb;
            pos    +=   2;
            break;
    }

    set_pos (pos);
    return ((int)ch);
}

/* 
 * set position 
 * return  0 for bad 
 *         1 for ok                    
 **/

inline int ksetpt ( __u32 posx,__u32 posy ) 
{
    unsigned long pos = console[cur_con_nr].curpos;
    pos = (posx + posy * 80)*2;
    
    console[cur_con_nr].curpos = pos;
    return (pos < 0x8000);
}

/*
 *  get position 
 *  return  0 for bad 
 *          1 for ok 
 */

inline int  kgetpt ( __u32* posx,__u32* posy ) 
{
    unsigned long pos = console[cur_con_nr].curpos;
    *posy = pos / 160 ;
    *posx = pos % 160 ;
    return (pos < 0x8000);
}

inline __byte ksetrgb ( __byte crgb )
{
    return (console[cur_con_nr].rgb = crgb);
}

inline __byte kgetrgb ( void ) 
{
    return (console[cur_con_nr].rgb);
}

/* clear screen */
void cls_con0 ( void )
{
    char *p = (char *)(0xb8000);
    int i = 0 ;

    while ( i ++ < 4000 )
    {
        p[i*2] = ' ';
        p[i*2+1] = 0x0F;
    }
}
