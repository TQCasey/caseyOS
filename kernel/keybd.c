#include <casey/types.h>
#include <casey/mm.h>
#include <casey/kernel.h>
#include <casey/termio.h>
#include <casey/sched.h>
#include <string.h>


/* 
 * well ... casey kb interrupt handle the in/out que keyboard buffer only 
 * which excuted by the kernel
 * 2012 -05 - 07 By Casey
 */

KBD_QUE *kb_buf  = (KBD_QUE*)(TERM_BUFFER); 


static __u8  kb_get ( __u8 elem  ) 
{
    kb_buf->locked = true;
	if ( (kb_buf->tail + 1) % (KB_BUF_LEN) == kb_buf->head )
		return (-1);
	kb_buf->tail = (kb_buf->tail + 1) % (KB_BUF_LEN);
    kb_buf->buf [ (int)(kb_buf->tail) ] = elem ;     //Low 8 bits
    kb_buf->len = (kb_buf->tail - kb_buf->head + KB_BUF_LEN) % (KB_BUF_LEN); 
    kb_buf->locked = false;
    return (0);
}

extern __u32 old_t_id;
/* kernel just store the scan code from keyboard */
static void keybd ( void ) 
{
    unsigned char elem ; 
    kb_get (elem = rdport (0x60)) ;
    /* wake up tty decode thread */
    wake_up (TASK_TTY,1);
}

void init_keybd ( void ) 
{   
    memset (kb_buf,0,0x1000);           /* map must be 4KB alignment */
    set_irq_prc (0x01,(void*)keybd);
}
