#include <unistd.h>
#include <stdio.h>
#include <casey/types.h>
#include <casey/termio.h>
#include <casey/mm.h>
#include <stdarg.h>
#include <casey/sched.h>
#include <sys/msgno.h>
#include "keys.h"

#define wrport(port,value) \
    __asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

extern CHAR_BUF *ch_buf ;
COOKED   *cooked_buf;

char chars[100] ;

static unsigned char rdport(unsigned short port) 
{ 
    unsigned char ret;
    __asm__  ("inb %%dx,%%al":"=a" (ret):"d" (port));
    return (ret);
}

static X *x = (X*)(ROCKECT);
static CONSOLE *console = NULL;
static unsigned char cur_con_nr  = 0;

/* tty write tty 2  */
void con_write ( void )
{
    MAIL m;
    

    while (true) pause ();

    /*
    while ( true ) 
    {
        pickm (&m);
        switch (m.umsg)
        {
            case TTYM_READ:
                while ( !(*chars) ) pause ();
                m.pkg[0] = (unsigned long)(chars);
                m.pkg[1] = 100;
                replym (&m);
                chars[0] = '\0';
                break;
        }
    }
    */

    /* should not arrive here ! */
    end ();
}

/* tty read */
void con_read ( void ) 
{
    while ( true ) 
    {
        pause ();
    }
    end ();
}

void setcurpos ( __u32 curpos ) 
{
    wrport (CRT_ADDR,CRT_CURPOS_H);
    wrport (CRT_DATA,(curpos>>9)&0xFF );
    wrport (CRT_ADDR,CRT_CURPOS_L);
    wrport (CRT_DATA,(curpos>>1)&0xFF );
}

CONSOLE* get_curcon ( void ) 
{
    return ( &console[cur_con_nr] ); 
}
static
CONSOLE* set_curcon ( unsigned char cur_con ) 
{
    x->cur_con_nr = cur_con ;
    return ( &console[cur_con] );
}

static 
void set_start_addr ( unsigned long addr ) 
{
    wrport (CRT_ADDR,CRT_START_H);
    wrport (CRT_DATA,(addr>>9) & 0xFF );
    wrport (CRT_ADDR,CRT_START_L);
    wrport (CRT_DATA,(addr>>1) & 0xFF );
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
    console[cur_con_nr].curpos = pos;
    setcurpos (pos);

    if ( (pos / 160) * 160 - console[cur_con_nr].cur_start_addr >= (4000) ) 
        scroll_down (1);
}

int  xputch ( const char ch ) 
{
    long pos = console[cur_con_nr].curpos;
    __byte *vbase = (__byte*)(STDOUT_BUF+pos),i = 0;
    unsigned char rgb = console[cur_con_nr].rgb;
    //unsigned long max = console[cur_con_nr].max_addr;

    switch ( ch ) 
    {
        case '\n':
            pos = (pos/160+1) * 160 ;   
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



extern char ch_del ( void ) ;

#define M 100
int xputcmd ( const char cmd ) 
{
    long pos = console[cur_con_nr].curpos;
    __byte *vbase = (__byte*)(STDOUT_BUF+pos),i = 0;
    unsigned char rgb = console[cur_con_nr].rgb;
    //unsigned long max = console[cur_con_nr].max_addr;

    switch (cmd) 
    {
        case 0x00:  /* shift + UP */
            scroll_up (1);
            break;
        case 0x01:  /* UP */
            break;
        case 0x02:  /* shift + DOWN */
            scroll_down (1);
            break;
        case 0x03:  /* DOWN */
            break;
        case 0x04:  /* backspace */
            if ( pos - 2 < 0 ) return (-1);
            if ( ch_del () ) return (-1);
            pos -= 2;
            vbase[-2] = ' ';
            break;
        case 0x05:  /* left */
            if ( pos - 2 < 0 ) return (-1);
            pos -= 2;
            break;
        case 0x06:  /* left */
            pos += 2;
            break;
        case 0x07:  /* enter */
            pos = (pos/160+1) * 160 ;   /* new line */
            set_pos (pos);

            cooked_buf->locked = true;
            getchs (chars,100);
            unblock (2);
            cooked_buf->locked = false;

            return ((int)cmd);
        case 0x08:  /* tab */
            for ( i = 0 ; i < 4 ; i ++ )
            {
                *vbase ++ = ' ';
                *vbase ++ = rgb;
            }
            pos += 2 * 4;
            break;
        default:
            return (-1);
    }

    set_pos (pos);
    return ((int)cmd);
}

/* 
 * set position 
 * return  0 for bad 
 *         1 for ok                    
 */
inline int setpt ( __u32 posx,__u32 posy ) 
{
    unsigned long pos = console[cur_con_nr].curpos;
    pos = (posx + posy * 80)*2;
    return (pos < 0x8000);
}

/*
 *  get position 
 *  return  0 for bad 
 *          1 for ok 
 */

inline int  getpt ( __u32* posx,__u32* posy ) 
{
    unsigned long pos = console[cur_con_nr].curpos;
    *posy = pos / 160 ;
    *posx = pos % 160 ;
    return (pos < 0x8000);
}

inline __byte setrgb ( __byte crgb )
{
    return (console[cur_con_nr].rgb = crgb);
}

inline __byte getrgb ( void ) 
{
    return (console[cur_con_nr].rgb);
}

int  xputs ( const char* str ) 
{
    char *p = (char*)str;
    unsigned long cnt = 0;
    while ( *p ) {
        cnt ++;
        xputch (*p++);
    }
    return (cnt);
}

int printx ( const char* fmt ,... ) 
{
    int len = 0 ;
    char buf[1024] = {0};

    va_list va_p = NULL;

    va_start (va_p,fmt);
    len = vsprintf (buf,fmt,va_p);
    len = xputs (buf);
    va_end (va_p);
    return ( len );
}

extern void decode_scan_code ( void ); 
extern void con_write ( void ) ;
extern void con_read ( void ) ;

/* tty_init */
void Init_TTY ( void ) 
{
    /* base =  TERM_BUFFER 
     * off = 0 - SIZE(KBD_QUE) for kbd_que
     * off = SIZE(KBD_QUE) for char_buffer
     */
    console = x->console;
    cur_con_nr = x->cur_con_nr;

    ch_buf      = (CHAR_BUF*)(STDIN_BUF + SIZE(KBD_QUE));                      
    ch_buf->head = ch_buf->tail = ch_buf->len = 0;

    cooked_buf = (COOKED*)(STDIN_BUF + SIZE(KBD_QUE) + SIZE(CHAR_BUF));   /* 1052 bytes */

    add_thread (decode_scan_code,TS_RUNNING);   /* 1 -- for analysize scan code */
    add_thread (con_write,TS_RUNNING);          /* 2 -- for tty write */
    add_thread (con_read,TS_RUNNING);           /* 3 -- for tty read */
}
