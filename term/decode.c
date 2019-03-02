#include <unistd.h>
#include <stdio.h>
#include <casey/types.h>
#include "keys.h"
#include <sys/msgno.h>
#include <casey/procs.h>
#include <casey/termio.h>
#include <casey/mm.h>
#include <casey/sched.h>
#include <string.h>


static X *x = (X*)(ROCKECT);
KBD_QUE *kb_buf     = (KBD_QUE*)(STDIN_BUF);
CHAR_BUF *ch_buf    = NULL;

#define cli()   __asm__("cli")
#define sti()   __asm__("sti")

extern int xputch ( char ch );
extern int xputcmd (char cmd );

static __u8 kb_put ( void  )
{
	/* no more scan code comes */
    cli ();
	while ( kb_buf->head == kb_buf->tail ) pause ();
	kb_buf->head = (kb_buf->head + 1) % (KB_BUF_LEN);
	kb_buf->len = (kb_buf->tail - kb_buf->head + KB_BUF_LEN) % (KB_BUF_LEN); 
    sti ();
	return ( (kb_buf->buf[ (int)(kb_buf->head) ])  );
}

static bool fcaps       = false;                         //caps lock 
static bool flctl       = false;                         //ctrl (left)
static bool frctl       = false;                         //ctrl (right)
static bool flshft      = false;                         //shift(left)
static bool frshft      = false;                         //shift(right)
static bool flalt       = false;                         //alter(left)
static bool fralt       = false;                         //alter(right)
static bool fE0         = false;                         //
static bool fnmlock     = false;                         //number lock 
static bool fscroll     = false;                         //scroll lock 

//set caps lock or unlock 
void setcaps ( bool fval  ) 
{
    fcaps = fval;
}

//get caps lock state
bool getcaps ( void ) 
{
    return (fcaps);
}

//set ctrl state
void setctl ( bool fleft ,bool fval )
{   
    bool *p = NULL;
    *(p = fleft ? (&flctl) : (&frctl)) = fval ;
}

//get ctrl state
bool getctl ( bool fleft ) 
{
    return ( fleft ? (flctl):(frctl) );
}

//set shift
void setshft ( bool fleft ,bool fval )
{
    bool *p = NULL;
    *(p = fleft ? (&flshft) : (&frshft)) = fval ;
}

//get shift state
bool getshft ( bool fleft ) 
{
    return ( fleft ? (flshft) :(frshft) );
}

//set alter
void setalt ( bool fleft ,bool fval )
{
    bool *p = NULL;
    *(p = fleft ? (&flalt) : (&fralt)) = fval ;
}

//get alter state
bool getalt ( bool fleft  ) 
{
    return ( fleft ? (flalt) : (fralt) );
} 

#define MAX_LEN MAX_CHAR_BUF_LEN
static 
void ch_in ( char ch ) 
{
    cli ();
	if ( ch_buf->head == ((ch_buf->tail+1) % MAX_LEN) ) 
    {
        sti ();
		return ;
    }
	ch_buf->tail = (ch_buf->tail + 1) % MAX_LEN;
	ch_buf->len  = (ch_buf->tail - ch_buf->head + MAX_LEN) % MAX_LEN;
	ch_buf->buf [ ch_buf->tail ] = ch;
    sti ();
}

static 
char ch_out ( void ) 
{
    cli ();
	if ( ch_buf->head == ch_buf->tail ) 
    {
        sti ();
		return (0);
    }
	ch_buf->head = (ch_buf->head + 1) % MAX_LEN;
	ch_buf->len  = (ch_buf->tail - ch_buf->head + MAX_LEN) % MAX_LEN;
    sti ();
	return ( ch_buf->buf[ch_buf->head] );
}

char ch_del ( void ) 
{
    cli ();
	if ( ch_buf->head == ch_buf->tail ) 
    {
        sti ();
		return (1);
    }
	ch_buf->tail = (ch_buf->tail + MAX_LEN - 1) %MAX_LEN ;
	ch_buf->len  = (ch_buf->tail - ch_buf->head + MAX_LEN ) % MAX_LEN;
    sti ();
    return (0);
}

char* getchs ( char *buf,long maxlen ) 
{
    if ( !buf || !(maxlen) ) return (NULL) ;

    int i = 0 ;
    for ( i = 0 ; i < Min (maxlen,MAX_LEN) ; i ++ )
        *buf++ = ch_out ();

    return (buf);
}

static __u8 col = 0 ;

static 
void func_key ( __u32 key ,bool make)
{
    switch ( key ) 
    {
        case SHIFT_L:   setshft (true ,make);   break;
        case SHIFT_R:   setshft (false,make);   break;
        case CTRL_L:    setctl  (true ,make);   break;
        case CTRL_R:    setctl  (false,make);   break;
        case ALT_L:     setalt  (false,make);   break;
        case ALT_R:     setalt  (true ,make);   break;
        default:        break;
    }
}

static 
void make_prc (__u8 make,__u32 key)
{
    char ch = 0,cmd_code = -1;
    key |= flshft  ? FLAG_SHIFT_L : 0 ;
    key |= frshft  ? FLAG_SHIFT_R : 0 ;
    key |= flctl   ? FLAG_CTRL_L  : 0 ;
    key |= frctl   ? FLAG_CTRL_R  : 0 ;
    key |= flalt   ? FLAG_ALT_L   : 0 ;
    key |= fralt   ? FLAG_ALT_R   : 0 ; 

    if ( !(key & FLAG_EXT) )// visiable
    {
        ch = (unsigned char)key;
    }
    else
    {
        switch ( key & MASK_RAW )
        {
            case UP:
            {
                if ( (key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R) )
                    cmd_code = 0x00;
                else
                    cmd_code = 0x01;
                ch = -2;
            }
            break;
            case DOWN:          
            {
                if ( (key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R) )
                    cmd_code = 0x02;
                else
                    cmd_code = 0x03;
                ch = -2;
            }
            break;
            case BACKSPACE:     ch = -2  ;cmd_code = 0x04;  break;
            case LEFT:          ch = -2  ;cmd_code = 0x05;  break;
            case RIGHT:         ch = -2  ;cmd_code = 0x06;  break;
            case ENTER:         ch = -2  ;cmd_code = 0x07;  break;
            case TAB:           ch = -2  ;cmd_code = 0x08;  break;
            case ESC:           ch = -2  ;cmd_code = 0x09;  break;
            default:            ;return;   // not recognised char 
        }
    }

    if ( ch != (-2) )
    {
        ch_in (ch);
        xputch (ch);
    }
    else   
        xputcmd (cmd_code);
}

/* analysise raw code */
void  decode_scan_code ( void )
{
    __u8 scan = 0;
    __u32 *prow = NULL,key = 0,i = 0  ;
    bool make = false,fpausebrk = false ;

    while ( true )
    {
        scan = kb_put () ;
        key = fE0 = 0;
        switch ( scan )
        {
            case 0xE1:
                for ( i = 1 ; i < 6 ; i ++ ) 
                {
                    if ( (scan = kb_put ()) != pausebrk[i]  ) 
                    {
                        fpausebrk = false ;
                        break;
                    }
                }
                if ( fpausebrk == true )
                    key = PAUSEBREAK ;
                break;
            case 0xE0:
                //when the print screen key down 
                if  ( (scan = kb_put()) == prtscrn_down[1] ) 
                {
                    if ((kb_put () == prtscrn_down[2]) && (kb_put () == prtscrn_down[3]) )
                    {
                        make = true;
                        key  = PRINTSCREEN;
                    }
                }
                //when the print screen key up
                if ( make  != true ) 
                {
                    if  ( scan == prtscrn_up[1] ) 
                    {
                        if ((kb_put () == prtscrn_up[2]) &&(kb_put () == prtscrn_up[3]) )
                            key  = PRINTSCREEN;
                    }
                }
                fE0 = (key ==0);
                break;
        }

        if (key != PAUSEBREAK && key != PRINTSCREEN  ) 
        {
            col = 0;
            make = (scan >> 7 == 0);
            if ( flshft || frshft )  col = 1;
            if ( fE0 == true )
            {
                col = 2;
                fE0 = false;
            }
            prow = (__u32*)&keymap[ (scan&0x7F) * MAP_COLS ];
            func_key (key = prow [col],make) ;
            /* prc make code */
            if ( make ) make_prc (make,key); 
        }
    }

    end ();
}
