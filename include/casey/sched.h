#ifndef __SCHED_H__
#define __SCHED_H__

#include <casey/procs.h>

//port definations
#define  TIMER0         0x40
#define  TIMER1         0x41
#define  TIMER2         0x42

#define  TMOD           0x43
#define  NRML_INIT      0x34
#define  TFRQ           1193180L
#define  HZ             100


#define TS_RUNNING      0
#define TS_STUCK        1
#define TS_SENDING      2
#define TS_RECVING      3
#define TS_WAIT_INTR    4       /* wait for specified interrupt */
#define TS_LOOP_WAIT    5       /* mail sendlist loop to wait */
#define TS_FULL_WAIT    6       /* mailbox full to wait */
#define TS_WAIT_CHILD   7       /* call sys_wait */
#define TS_EXITED       8


#define MAX_PROCS       64
extern PROCESS*         procs_ptr[ MAX_PROCS ] ;

#define MAX_TASKS               5
#define INIT                    1
#define FS                      2
#define TTY                     3
#define MM                      4
#define SHELL                   5

#define TASK_INIT       procs_ptr[INIT]
#define TASK_FS         procs_ptr[FS]
#define TASK_TTY        procs_ptr[TTY]
#define TASK_MM         procs_ptr[SHELL]

extern unsigned char    NR_TASKS;
extern PROCESS*        __curprocs;
extern unsigned char    NR_USR_PROCS;
extern void schedule ( void ); 
extern void wake_up  ( PROCESS *dest,short thread_nr );
extern void sleep_on ( PROCESS *dest,short thread_nr );

#endif


