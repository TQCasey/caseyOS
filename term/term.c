#include <unistd.h>
#include <stdio.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <sys/msgno.h>
#include <casey/mm.h>
#include <casey/termio.h>
#include <casey/sched.h>


extern void Init_TTY (void);

/* main thread for Service */
int main ( void ) 
{   
    Init_TTY ();

    MAIL m;
    m.m_msg = 1111;
    m.m_dest_pid    = INIT;
    m.m_level       = MPL_USR;
    m.m_dest_tid    = 0;


    delay_ms (1000);

    pause ();

    /*
    int i = 0;
    while ( true ) {
        delay_ms (10);
        postm (&m);
        printf ( "[%d]" ,i++ );
    }
    */


    while ( true ) pause ();
}
