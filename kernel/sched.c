#include <casey/types.h>
#include <casey/sched.h>
#include <casey/procs.h>
#include <casey/kernel.h>
#include <casey/mm.h>

/*
 * we must get the greatest ticks remained in the procs
 * so first we need put their ticks in sort and place 
 * in a line ,I hope it works !!
 */

/* ONLY can switch_to  () change cur_tid */
u32   __ticks;
short cur_tid;
extern __u8     NR_TASKS,NR_USR_PROCS;
extern __byte   __k_reenter;

/* clock handler */
static void clock ( void ) 
{
    __curprocs->thread[cur_tid].state  = TS_RUNNING;
    __ticks  ++;
    /*
     * we need set up a maximumise num to indentify how much times 
     * we can get access to the kernel,'cause the kernel stack has 
     * limit spaces,so we chk this before doing something in the kernel
     */
    if ( __k_reenter > MAX_ENTER_TIMES ) 
        return ;
    if ( __curprocs->thread[cur_tid].curtks >= 0 )
       __curprocs->thread[cur_tid].curtks -- ;         //current ticks -- when get the timepiece
    /* 
     * this is short time schedule ,It should not take too much time 
     * less than 10ms is better 
     */
    schedule ();                                        //do processes schedule module
}


/* init 8253 */
static void Init8253 ( void )
{
      wrport (TMOD   ,NRML_INIT);
      wrport (TIMER0 ,(unsigned char)(TFRQ/HZ));        //high 8 bits
      wrport (TIMER0 ,(unsigned char)((TFRQ/HZ)>>8));   //Low  8 bits
}

void init_clock ( void ) 
{
    Init8253 ();
    set_irq_prc ( 0x00,(void*)clock );
}

extern PROCESS task0;
static PROCESS *prev_proc = &task0;
static short prev_tid = 0;
extern unsigned char __switched;

/* switch to specified thread */
void switch_to (PROCESS *proc,short tid ) 
{
    /* do nothing ...*/
    if ( __curprocs == proc && tid == cur_tid )  return ;
    /* ==================== notes ======================
     * soft ware intr can change eax as return value ,while hard
     * ware interrupt can not,so a flag to identify the intr src
     * kind is necessary ! 
     * val::sotf_intr_flag  
     * @ 0 : useless intr kind 
     * @ 1 : as hard ware intr kind 
     * @ 2 : as soft ware intr kind 
     *=========================================================*/
    THREAD_REGS *regs_1,*regs_2;

    /* save old thread relatively regs */
    regs_1 = (THREAD_REGS*)(&(__curprocs->thread[cur_tid].edi));
    regs_2 = (THREAD_REGS*)(&(__curprocs->regs.edi));

    *regs_1 = *regs_2;
    __curprocs->thread[cur_tid].soft_intr = __curprocs->soft_intr_flag;

    /* switched flags set */
    __switched  = true;
    prev_proc   = __curprocs;
    prev_tid    = cur_tid;

    /* load new regs all */
    cur_tid    = tid;
    __curprocs  = proc;

    /* load new  thread relatively regs */
    regs_1 = (THREAD_REGS*)(&(__curprocs->thread[cur_tid].edi));
    regs_2 = (THREAD_REGS*)(&(__curprocs->regs.edi));

    *regs_2 = *regs_1;
    __curprocs->soft_intr_flag = __curprocs->thread[cur_tid].soft_intr;
}


static
void choose_max ( PROCESS **proc,short *thread_nr ) 
{
    short   max_proc = 0,max_thread = 0,i = 0 ,j = 0 ;
    /* this is the thread based schedule ,It might be complicated 
     * first ,we need enumulate all thread in each process and 
     * find a greatest one !
     * TASK 0 thread 0 can not be stucked ,even use pause () sys call 
     * the rest threads of TASK 0 is okay !
     */
    for ( i = 0 ; i < NR_TASKS + NR_USR_PROCS; i ++ ) 
    {
        if ( !procs_ptr[i] )continue;
        for ( j = 0 ; j < procs_ptr[i]->thread_nr  ; j ++ )
        {
            if ( procs_ptr[i]->thread[j].state != TS_RUNNING )continue ;
            /* find the greatest tks */
            if ( procs_ptr[i]->thread[j].curtks > 
                    procs_ptr[max_proc]->thread[max_thread].curtks ) 
            {
                max_proc = i;
                max_thread = j;
            }
        }
    }
    /* all process thread tks is run out ,then we need to reload the tks*/
    if ( procs_ptr[max_proc]->thread[max_thread].curtks <= 0 )
    {
        /* reloadary ignore the thread state ,enven it is stuck 
         * no problem here ,but another schedle () will handle 
         * this state !!!!!!!!!!
         */
        for (i = 0 ; i < NR_TASKS + NR_USR_PROCS ; i ++ )
        {
            if ( !procs_ptr[i] )continue;
            for ( j = 0 ; j < procs_ptr[i]->thread_nr ; j ++ )
                procs_ptr[i]->thread[j].curtks = procs_ptr[i]->thread[j].pl;
        }
        for ( i = 0 ; i < NR_TASKS + NR_USR_PROCS; i ++ ) 
        {
            if ( !procs_ptr[i] )continue;
            for ( j = 0 ; j < procs_ptr[i]->thread_nr ; j ++ )
            {
                if ( procs_ptr[i]->thread[j].state != TS_RUNNING )continue ;
                /* find the greatest tks */
                if ( procs_ptr[i]->thread[j].curtks > 
                        procs_ptr[max_proc]->thread[max_thread].curtks ) 
                {
                    max_proc = i;
                    max_thread = j;
                }
            }
        }   
    } 
    *proc = procs_ptr[max_proc];
    *thread_nr = max_thread;
}

void schedule ( void ) 
{
    /* ok ,now start to change the entry now ,GOD BLESS ME ! */
    PROCESS *proc;
    short max_thread_nr;

    choose_max (&proc,&max_thread_nr);
    switch_to (proc,max_thread_nr);
}


/* can make every thread sleep */
void sleep_on ( PROCESS *dest,short thread_nr )
{
    if ( !dest ) return ;
    if ( dest->thread[thread_nr].state == TS_STUCK ) return ;
    dest->thread[thread_nr].state   = TS_STUCK;
    schedule ();
}


/* can wake up every thread */
void wake_up ( PROCESS *dest,short thread_nr )
{
    if ( !dest ) return ;
    if ( dest->thread[thread_nr].state == TS_RUNNING ) return ;
    dest->thread[thread_nr].state   = TS_RUNNING;
    switch_to (dest,thread_nr);
}

/* self stuck sys call */
u32 sys_pause (void) 
{
    sleep_on (__curprocs,cur_tid);
    return (0xDF);
}

/* 
 * any way scheduled thread will come to this place 
 */
void ret_prc ( u32 prev_tid_ret_val ) 
{
    /* if prev procs is intred from syscall 
     * we need store this pre_tid_ret val to 
     * prev procs thread regs ,it will be safe 
     */
    if ( !__switched ) 
    {
        /* if need return val */
        if ( __curprocs->soft_intr_flag == 2 ) 
            __curprocs->regs.eax = prev_tid_ret_val ;
    }
    else    /* if switched ,prev_proc & prev_tid is validate */
    {
        /* if prev_proc need ret ,then store the return val to ret slot */
        if ( prev_proc->thread[prev_tid].soft_intr == 2 )
            prev_proc->thread[prev_tid].t_ret_val = prev_tid_ret_val;
        /* if current proc need return ,then load retval */
        if ( __curprocs->soft_intr_flag == 2 ) 
            __curprocs->regs.eax = __curprocs->thread[cur_tid].t_ret_val;
    }
}
