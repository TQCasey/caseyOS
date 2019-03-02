#include <sys/msgno.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>

extern u32 __ticks;
extern i16 cur_tid;

/* send mail send mail to dest process && thread 
 */
u32 sys_postmail ( MAIL *pmail )
{
    /* nil mail denied !*/
    if ( NULL == pmail ) {
        printk ( "post NULL mail !\n" );
        return (-1);
    }

    i16 src_pid,src_tid,dest_pid,dest_tid ;
    src_pid = __curprocs->pid,src_tid = cur_tid;
    /* relocate */
    pmail = (MAIL*) ((u32)(pmail) + (src_pid * USER_SPACE));    /* mail relocate */

    /* get src dest pids,tids */
    dest_pid = pmail->m_dest_pid,dest_tid = pmail->m_dest_tid;
    u8  max_threads = procs_ptr[dest_pid]->thread_nr,
                   *state = (u8*)(&(procs_ptr[dest_pid]->thread[dest_tid].state));
    /* chking ... */
    if ( !dest_pid                      /* task 0 has no mailbox */
        || dest_tid >= max_threads 
        || !procs_ptr[dest_pid] )       /* dest process is validate */
    {
        printk ( "trying to send mail to wrong thread !\n" );
        return (-1);
    }
    /* loop sent denied ! */
    if ( dest_pid == src_pid && dest_tid == src_tid ) {
        printk ( "self post mail loop denied !\n" );
        return  (-1);
    }
    /* special tid denied ! */
    if(dest_pid == TTY && dest_tid == 1) {/* TTY decode */
        printk ( "special thread post mail denied ! \n" );
        return (-1);
    }
    u8 *count = NULL;
    MAIL *mail = find_empty_mail_slot (0,dest_pid,dest_tid,
                                            pmail->m_level,&count);
    if (NULL == mail){  /* not found */
        MAILBOX *mailbox = (MAILBOX*)(procs_ptr[dest_pid]->mail_box);
        __curprocs->thread[cur_tid].state = TS_FULL_WAIT ;  /* post mail slot is full */ 

        if ( !mailbox->post.head )
            mailbox->post.head = mailbox->post.tail = pmail;
        else 
            (mailbox->post.tail)->m_next = pmail;

        pmail->m_next = NULL;
        mailbox->post.cnt ++;

        schedule ();            
        return (-1);
    }

    /* copy message */
    mail->m_locked      = mail->m_validate = true;
    mail->m_atime       = __ticks ;
    mail->m_src_pid     = src_pid ; mail->m_src_tid = src_tid;
    mail->m_dest_pid    = dest_pid; mail->m_dest_tid = dest_tid;
    mail->m_level       = pmail->m_level;
    mail->m_msg         = pmail->m_msg;
    mail->m_fparam      = pmail->m_fparam;
    mail->m_lparam      = pmail->m_lparam;
    mail->m_type        = M_WAY_POST;   
    mail->m_ret_type    = pmail->m_ret_type;
    mail->m_data_d      = 0;
    mail->m_next        = NULL;
    (*count) ++;

    /* wake up recving mail thread if there is one 
     */
    if ( *state == TS_RECVING ) 
        *state = TS_RUNNING;
    /* not schedule it ,just let dest procs to chk mail*/
    return (0);
} 
