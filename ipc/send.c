#include <sys/msgno.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>

extern u32 __ticks;
extern i16 cur_tid;


/* loop waited send mail detected ! */
static 
char try_to_send (MAIL *pmail)
{
    i16 src_pid,src_tid,dest_pid,dest_tid ;
    /* get src dest pids,tids */
    src_pid = __curprocs->pid,src_tid = cur_tid;
    dest_pid = pmail->m_dest_pid,dest_tid = pmail->m_dest_tid;

    i16 next_pid,next_tid,temp_pid,temp_tid;

    next_pid = dest_pid,next_tid = dest_tid;
    while ( 1 ) {
        temp_pid = next_pid,temp_tid = next_tid;    /* temp load */

        if ( !procs_ptr[temp_pid] ) return (0);
        if ( !procs_ptr[temp_pid]->thread[temp_tid].taken ) return (0);

        next_pid = procs_ptr[temp_pid]->thread[temp_tid].send_q.send_to_pid;
        next_tid = procs_ptr[temp_pid]->thread[temp_tid].send_q.send_to_tid;

        if ( (-1) == next_pid && (-1) == next_tid ) {/* send to noboby */
            procs_ptr[src_pid]->thread[src_tid].send_q.from_pid = src_pid;
            procs_ptr[src_pid]->thread[src_tid].send_q.from_tid = src_tid;

            procs_ptr[src_pid]->thread[src_tid].send_q.send_to_pid = dest_pid;
            procs_ptr[src_pid]->thread[src_tid].send_q.send_to_tid = dest_tid;
            return (0);     /* no loop */
        }
        if ( src_pid == next_pid && src_tid == next_tid ) 
            return (1);     /* has mail send loop */
    }
}

/* post mail () sync mail send... ONLY wake up recving mail thread .... 
 */
u32 sys_sendmail ( MAIL *pmail )
{   
   if ( NULL == pmail ) {
        printk ( "send NULL mail !\n" );
        return (-1);
    }
    /* nil mail denied !*/
    i16 src_pid,src_tid,dest_pid,dest_tid ;
    src_pid = __curprocs->pid,src_tid = cur_tid;

    /* relocate */
    pmail = (MAIL*) ((u32)(pmail) + (src_pid * USER_SPACE));

     /* get src dest pids,tids */
    dest_pid = pmail->m_dest_pid,dest_tid = pmail->m_dest_tid;
    u8  max_threads = procs_ptr[dest_pid]->thread_nr;

    /* chking ... */
    if ( !dest_pid                      /* task 0 has no mailbox */
        || dest_tid >= max_threads 
        || !procs_ptr[dest_pid] )
    {
        printk ( "trying to send mail to wrong thread !\n" );
        return (-1);
    }
    /* chk if there is some send loop */
    if ( dest_pid == src_pid && dest_tid == src_tid ) {
        printk ( "send mail loop denied !\n" );
        return  (-1);
    }
    /* special tid denied ! */
    if (dest_pid == TTY && dest_tid == 1){  /* TTY decode not respond */
        printk ( "special thread post mail denied ! \n" );
        return (-1);
    }

    /* mail loop chk */
    if ( try_to_send (pmail) ) {    /* loop waited */
        printk ( "send mail loop waited !\n" );
        procs_ptr[src_pid]->thread[src_tid].state = TS_LOOP_WAIT;
    }
    else {
        u8 *count;
        MAIL *mail = find_empty_mail_slot (1,
                dest_pid,dest_tid,
                pmail->m_level,&count);
        
        if ( !mail->m_locked && !mail->m_validate ) {/* invalidate && open */
            mail->m_locked      = mail->m_validate = true;
            mail->m_atime       = __ticks ;
            mail->m_src_pid     = src_pid     ;mail->m_src_tid    = src_tid;
            mail->m_dest_pid    = dest_pid    ;mail->m_dest_tid   = dest_tid;
            mail->m_level       = pmail->m_level;
            mail->m_msg         = pmail->m_msg;
            mail->m_fparam      = pmail->m_fparam;
            mail->m_lparam      = pmail->m_lparam;
            mail->m_type        = M_WAY_SEND;         /* send mail (sync mail type */ 
            mail->m_ret_type    = pmail->m_ret_type;
            mail->m_data_d      = pmail->m_data_d;
            mail->m_next        = NULL;
            /* self stucked */
            __curprocs->thread[cur_tid].state = TS_SENDING;

            /* if dest (pid,tid) is not recving mail ,do nothing about it ! */
            if ( procs_ptr[dest_pid]->thread[dest_tid].state  == TS_RECVING) 
                procs_ptr[dest_pid]->thread[dest_tid].state  = TS_RUNNING; 
        }else {
            MAIL *tmp = mail;
            while ( tmp ) tmp = tmp->m_next;
            tmp->m_next = pmail;        /* attach to the tail */
        }
        (*count) ++;
    }
    schedule ();
    return (0);
}

