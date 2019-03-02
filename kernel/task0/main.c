#include <unistd.h>
#include <casey/types.h>
#include <sys/types.h>
#include <string.h>
#include <casey/procs.h>

void main ( void ) 
{
    cls_con0 ();

    printk ( "Hard disk drv number : %d \n",getb(0x475));  /* printk () not problem here */

    startup ();     /* start up task0 */

    while (true)    pause ();   /* for schedule () */
}

