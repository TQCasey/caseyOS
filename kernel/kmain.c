#include <casey/types.h>
#include <sys/types.h>
#include <casey/procs.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>
#include <casey/dtcntl.h>

/* first task */
PROCESS*  procs_ptr[MAX_PROCS] = {0};
PROCESS*  __curprocs      = NULL;
u8    NR_TASKS;
u8    NR_USR_PROCS;

extern void init_clock ( void ) ;
extern void init_mm (void);
extern void init_exceptn ( void ) ;
extern void init_hd ( void ) ;
extern void init_console ( void ) ;
extern void init_sys_calls ( void );
extern void init_keybd ( void );
extern void init_task0 (void);
extern unsigned long sys_install (short pid);
extern void __procs_restart ( void ) ;  
extern unsigned long __ticks ;
extern short cur_tid;

/* kernel main entry */
void __kernel_main ( void ) 
{
    __ticks = 0;
    cur_tid = 0;;

    init_console ();
    init_exceptn ();

    memset (&procs_ptr,0,sizeof(procs_ptr));
    //irq call install...
    init_keybd ();
    //init sys_calls
    init_sys_calls ();
    //init mm module
    init_mm ();

    //init hard disk intr
    init_hd ();

    /* start to init first task */
    /* basic tasks */
    init_task0 ();        /* task0 */
    NR_USR_PROCS = 0 ;

	__curprocs  = procs_ptr[0] ;	               
    init_clock ();

	//don't touch it!!.
	__procs_restart ();     
    while ( true );         /* should not to be executed ! */
}
