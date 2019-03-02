#include <stdio.h>
#include <unistd.h>
#include <casey/dtcntl.h>
#include <casey/procs.h>
#include <casey/sched.h>
#include <casey/mm.h>


extern short cur_tid;

/* wait for latest forked child thread to exit 
 * 'cause the forked process tid must be 0
 * so just need to check the dest proces->thread[0]
 */

unsigned long sys_waitpid (short pid,unsigned char *state)
{
    unsigned char *offset ;

    offset = (unsigned char*)(__curprocs->pid * USER_SPACE + (unsigned long)state);

    *offset = 1;

    /* find child pid,tid && chk it's state (TS_EXITED ? )
     * if already exited ,then return back right away ,or 
     * parent need wait for child to exit
     */

    /* first ,find pid,tid in proc tables */
    int i,j ;
    THREAD  t;
    for ( i = 1 ; i < NR_TASKS + NR_USR_PROCS ; i ++ ) 
    {
        if ( !procs_ptr [i]  ) continue;
        if ( procs_ptr[i]->thread[0].father.pid == pid ) 
        {
            if (procs_ptr[i]->thread[0].state == TS_EXITED)
            {

            }
        }
    }
    
    __curprocs->thread[cur_tid].state = TS_WAIT_CHILD; 
    schedule ();

    return (0);
}
