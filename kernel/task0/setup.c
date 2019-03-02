#include <casey/types.h>
#include <sys/types.h>
#include <casey/procs.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>
#include <casey/dtcntl.h>
#include <casey/kernel.h>

/* first task */
PROCESS task0;

#define TASK0_STACK_SIZE    0x4000
static  __byte  task0_stack[TASK0_STACK_SIZE] = {0};

extern void main ( void ) ;
extern void InitProcsLdt ( PROCESS *proc );


/* the first task task0 process */
void init_task0 ( void ) 
{
    short pid = 0;
    /* nr_procos = 0 */
    procs_ptr[0]             = &task0;
    task0.pid                = pid ;                                            //procs id
    task0.mail_box           = 0 ; 

    task0.ldtsel             = SEL(INDEX_LDT_FIRST+pid);                        //ldt sel
    InitDT (&(task0.ldts[0]),0,0xFFFFF,DA_C|DA_32|(PL_TASK<<5)|DA_LIMIT_4K);    /* 0 - 4G */
    InitDT (&(task0.ldts[1]),0,0xFFFFF,DA_DRW|DA_32|(PL_TASK<<5)|DA_LIMIT_4K);

    //Selector SET           Ldt[index]         (MASK TI RPL)	    TI        RPL 
    task0.regs.cs            = ((SEL(INDEX_LDT_C) & SA_RPL_MASK) & SA_TI_MASK)| SA_TIL|RPL_TASK;
    task0.regs.ds            = 
    task0.regs.es            = 
    task0.regs.fs            = 
    task0.regs.ss            = (((SEL(INDEX_LDT_RW)) & SA_RPL_MASK) & SA_TI_MASK)| SA_TIL|RPL_TASK;
    task0.regs.gs            = ((SELECTOR_KERNEL_GS) & SA_RPL_MASK) | RPL_TASK ;
    task0.regs.eip           = (u32)(&main);
    task0.regs.esp           = (u32)task0_stack + TASK0_STACK_SIZE ;//Point to top
    task0.regs.eflags        = 0x1202;   

    /* main thread task0.... */
    THREAD *p = &(task0.thread[0]);
    p->father.pid           = p->father.tid = -1;   /* no father */
    p->cs                   = (SEL(INDEX_LDT_C) & SA_RPL_MASK & SA_TI_MASK)| SA_TIL|RPL_TASK;
    p->curtks               = p->pl  = 1;
    p->eip                  = (u32)(&main);
    p->esp                  = (u32)task0_stack + TASK0_STACK_SIZE;
    p->eflags               = 0x1202;
    p->state                = TS_RUNNING;
    p->t_id                 = 0;
    p->taken                = 1;
    p->send_q.send_to_pid   = p->send_q.send_to_tid = -1;   /* no send to */
    p->send_q.from_pid      = p->send_q.from_tid    = -1;   /* no src */
    task0.thread_nr         = 1;

    NR_TASKS ++;
    InitProcsLdt (&task0);
}

