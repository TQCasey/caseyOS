#include <casey/kernel.h>
#include <casey/sched.h>

extern unsigned char __k_reenter;
extern short old_t_id;
extern unsigned char intred;
extern short wait_tid;

/* this can be ok !,kernel && other thread hd intr rq is bad except 
 * for TASK_FS && tid (1) ,It runs well!  
 */
static void hdisk ( void ) 
{
    if ( __k_reenter > 0 )                  /* hd intr from kernel */ 
        return ;                            /* ignore */
    rdport (0x1F7);                         /* clear ISR */

    intred = 1;                             /* set intred */
    if ( -1 != wait_tid )
        wake_up (TASK_FS,wait_tid);
}


/* install hard disk intr */
void init_hd ( void ) 
{
    enable_irq (IRQ_CASCADE,true);          /* cascade is ready ! */ 
    enable_irq (IRQ_HD,true);
    set_irq_prc (14,(void*)hdisk);
}
