#include <casey/procs.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <casey/kernel.h>
#include <assert.h>
#include <string.h>

extern short cur_tid;
extern void free_page_dir_tables ( u32 page_addr,u32 page_dir_nr ) ;
extern void free_page ( u32 phys_addr ); 


static 
void release ( PROCESS *p ) 
{
    short pid = p->pid;
    /* free mailbox */
    free_page ((unsigned long)(p->mail_box));

    /* free pcb */
    free_page ((unsigned long)(procs_ptr[pid]));

    /* free page mem */
    free_page_dir_tables (pid *USER_SPACE,16);  /* free mem */

    /* free process slot */
    procs_ptr[pid] = NULL;

    /* rid off GDT table obj */
    memset((DESCRIPTOR*)(&GDT[pid+INDEX_LDT_FIRST]),0,sizeof(DESCRIPTOR));
}

/* kill it's self */
unsigned long sys_exit ( unsigned long exit_code ) 
{
    short pid = __curprocs->pid;
    short parent = __curprocs->thread[cur_tid].father.pid;
    int i = 0 ;
    PROCESS *p_ptr = procs_ptr[parent];

    /* dec PROC NRS  */
    if ( pid < MAX_TASKS ) {
        panic ( "trying to exit task proc !\n" );
        NR_TASKS --;
    }
    else
        NR_USR_PROCS --;

    /* to wake up parent wait pid if there is some one */
    if (parent == (unsigned short)(-1)) /* init no parent */
    {
        /* later modified ! */
        panic ( "system is shut down !\n" );
    }
    else
    {
        /* wake up waited thread */
        if ( p_ptr == NULL ) 
            panic ( "NULL parent pcb !\n" );
        for ( i = 0 ; i < p_ptr->thread_nr ; i ++ )
        {
            // found this pid in parent wait pid 
            //if ( p_ptr->thread[i].child_tid == pid )
            {
                p_ptr->thread[i].state = TS_RUNNING;
                break;
            }
        }
    }

    __curprocs->thread[cur_tid].state = TS_EXITED;
    /* now currrent pcb is destroyed by itself,but switch process calls 
     * use current ptr pcb ,so we need change this invlidate current 
     * pcb to a validate one !
     */
    __curprocs = p_ptr ; /* let it return from parent process */

    THREAD_REGS *regs_1,*regs_2;

    regs_1 = (THREAD_REGS*)(&(__curprocs->thread[cur_tid].edi));
    regs_2 = (THREAD_REGS*)(&(__curprocs->regs.edi));

    *regs_2 = *regs_1;
    __curprocs->soft_intr_flag = __curprocs->thread[cur_tid].soft_intr;

    return (0); /* thid ret_val is parent ret_val */
}
