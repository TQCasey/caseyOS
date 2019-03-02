#include <casey/types.h>
#include <sys/types.h>
#include <unistd.h>
#include <casey/kernel.h>
#include <casey/sched.h>
#include <casey/mm.h>

u32   NR_SYSCALL = 0;
PRC     __sys_call[ MAX_SYS_CALL ] = {0};

extern u32 __ticks;

static 
u32 sys_get_tks ( void ) 
{
    return (__ticks);
} 

extern u32 sys_startup (u32 addr);
extern u32 sys_exit (u32 exit_code);
extern u32 sys_kill (i16 pid);

static 
u32 sys_tty_write ( const char *str,u32 len  ) 
{
    u32 off = USER_SPACE * (__curprocs->pid);
    off += (u32)str ;
    char *p = (char*)off;
    
    while ( len --  ) kputch ( *p ++ );
    return ( p - str);
}

static 
u32 sys_tty_read ( u8 *buf,u32 len )
{
    return (0);
}

extern void sys_pause ( void );

static 
u32 sys_add_thread (void* thread_entry,u8 state )
{
    int i = 1;
    i16 pid  = __curprocs->pid;
    u8 rpl,pl ;
    u32 eflags ;

    if ( pid < MAX_TASKS ) {
        rpl = RPL_TASK;
        eflags = 0x1202;
        pl = 4;
    }
    else {
        rpl = RPL_USER;
        eflags = 0x202;
        pl = 3;
    }

    for ( i = 1; i < MAX_THREAD ; i ++ ) {
        if ( !__curprocs->thread[i].t_id ) {
            THREAD *p = &(__curprocs->thread[i]);
            p->cs                   = (SEL(INDEX_LDT_C) & SA_RPL_MASK & SA_TI_MASK)| SA_TIL|rpl;
            p->t_id                 = i;
            p->eip                  = (u32)thread_entry;
            p->esp                  = USER_SPACE  - 0x8000 * i;
            p->state                = state;
            p->send_q.send_to_pid   = -1;
            p->send_q.send_to_tid   = -1;
            p->send_q.from_pid      = -1;
            p->send_q.from_tid      = -1;
            p->pl                   = p->curtks    = pl;
            p->eflags               = eflags;
            p->taken                = 1;
            __curprocs->thread_nr ++;
            return (i);
        }
    }
    return (-1);
} 

extern i16 cur_tid;

/* NOTES 
 * end thread (id) doesn't free the thread stack mem page,
 * till the exit () is called 
 */

static 
u32  sys_end_thread ( i16 t_id ) 
{
    i16 pid = __curprocs->pid;
    i16 kill_tid = -1;

    if ( t_id == 0 ) return (-1);               /* thread 0 can not be killed ! */

    if ( t_id == -1 )                           /* self - kill */
        kill_tid = cur_tid;
    else {                                      /* kill specified tid */
        int i = 0 ; 
        for ( ; i < __curprocs->thread_nr ; i ++ ) 
        {
            if ( t_id == __curprocs->thread[i].t_id )
                kill_tid = t_id;
        }
        if ( kill_tid < 0 ) return (-1);        /* not found */
    }

    __curprocs->thread[kill_tid].t_id  = 0;     /* id slot  is empty now ! */
    __curprocs->thread[kill_tid].taken = 0;
    __curprocs->thread_nr --;                   /* nr -- */

    /* schedule () */
    if ( t_id == -1 ) 
        schedule ();

    return (0);
} 

extern i16 cur_tid;
static 
u32 sys_get_pinfo ( u32 *tasks_nr )
{
    if (tasks_nr == NULL)return (-1);

    u32 off = USER_SPACE * (__curprocs->pid);
    off += (u32)tasks_nr ;

    (*(u32*)(off)) = NR_TASKS;
    return ((__curprocs->pid << 16) | cur_tid );
}

u32 wait_tid = -1 ;
u8 intred = 0 ;

static 
u32 sys_wait_hd_intr ( void ) 
{
    if ( TASK_FS != __curprocs ) return (-1);

    if ( intred )
    {
        intred = 0; wait_tid = -1;
        return (-1);
    }

    if ( !intred ) 
    {
        wait_tid = cur_tid;                    /* get wait tid */

        if ( __curprocs->thread[cur_tid].state == TS_STUCK ) return (-1) ;
        __curprocs->thread[cur_tid].state   = TS_STUCK;
        schedule ();
        return (0);
    }
    return (-1);
}

/* nr = 9 */
static 
u32 sys_badness ( void ) 
{
    if ( __curprocs->pid < NR_TASKS ) 
    {
        cli ();
        while ( true );
    }
    return ( -1 ) ;
}

/* nr = 10 */
static 
u32 sys_block ( i16 tid ) 
{
    if (tid == (u32)(-1) || tid >= __curprocs->thread_nr ) return (-1);
    sleep_on (__curprocs,tid);
    return (0);
}

/* nr = 11 */
static 
u32 sys_unblock ( i16 tid ) 
{
    if (tid == (u32)(-1) || tid >= __curprocs->thread_nr ) return (-1);
    wake_up (__curprocs,tid);
    return (0);
}


extern u32 sys_waitpid (i16 pid,u8 *state );
#include <sys/msgno.h>

extern u32 sys_shell ( void ) ;
extern u32 sys_setup ( void ) ;
extern u32 sys_postmail ( MAIL *pmail );
extern u32 sys_sendmail ( MAIL *pmail );
extern u32 sys_pickmail ( MAIL *pmail,u8 flags );
extern u32 sys_replymail( MAIL *pmail );

/* install syscalls */
void init_sys_calls ( void  ) 
{
    /* msg calls */
    set_sys_call (__NR_postmail,sys_postmail);
    set_sys_call (__NR_sendmail,sys_sendmail);
    set_sys_call (__NR_pickmail,sys_pickmail);
    set_sys_call (__NR_replymail,sys_replymail);
    
    /* sys calls */
    set_sys_call (__NR_get_tks,sys_get_tks);
    set_sys_call (__NR_startup,sys_startup);
    set_sys_call (__NR_exit,sys_exit);
    set_sys_call (__NR_tty_write,sys_tty_write);
    set_sys_call (__NR_tty_read,sys_tty_read);
    set_sys_call (__NR_pause,sys_pause);
    set_sys_call (__NR_add_thread,sys_add_thread);
    set_sys_call (__NR_end_thread,sys_end_thread);
    set_sys_call (__NR_get_pinfo,sys_get_pinfo);
    set_sys_call (__NR_wait_hd_intr,sys_wait_hd_intr);
    set_sys_call (__NR_badness,sys_badness);
    set_sys_call (__NR_block,sys_block);
    set_sys_call (__NR_unblock,sys_unblock);
    set_sys_call (__NR_waitpid,sys_waitpid);
}

