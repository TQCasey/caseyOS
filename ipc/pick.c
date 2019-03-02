#include <sys/msgno.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>

extern u32 __ticks;
extern i16 cur_tid;

/* pick mail from mailbox */
u32 sys_pickmail ( MAIL *pmail,u8 flags )
{   
    if ( NULL == pmail ) {
        printk ( "wrong mail ptr ! \n" );
        return (-1);
    }

    pmail = (MAIL*) ((u32)(pmail) + (__curprocs->pid * USER_SPACE));
    MAILBOX *mailbox = (MAILBOX*)(__curprocs->mail_box);
    MAIL *mail;
    int roger = false;
    i16 dest_pid = __curprocs->pid,dest_tid = cur_tid;
    char *count;

    /* search mail from mailbox,if there is someone 
     * dest to this thread we pick it out of mailbox .
     * then dec the mail nr 
     */

    if ( mailbox->mb_snd_cnt > 0 )       /* some send mail come */
    {
        panic ( "send mail comes here !\n" );
        /* roger my send mail */
        //if ( (mail = find_spec_mail (1,dest_pid,dest_tid,0)) )
        {
       //     mailbox->m_send_mail_cnt --;        /* send mail roger */
         //   roger = true ;
        }
    }

    if ( !roger )   /* send mail comes ,but not mine @.@ */
    {   
        if ( mail = find_mb_mail (dest_pid,dest_tid,pmail->m_msg,&count,flags)) {
            (*count) --;
            roger = true;
        }
    }

    if ( roger )    /* roger mail */ 
    {
        i16 src_pid = mail->m_src_pid,src_tid = mail->m_src_tid;
        mail->m_locked = false;                 /* open */

        pmail->m_msg        = mail->m_msg;
        pmail->m_level      = mail->m_level;
        pmail->m_src_pid    = mail->m_src_pid;
        pmail->m_src_tid    = mail->m_src_tid;   /* src/dest */
        pmail->m_dest_pid   = mail->m_dest_pid;
        pmail->m_dest_tid   = mail->m_dest_tid; 
        pmail->m_type       = mail->m_type;      /* sync/async?  */

        if ( mail->m_type == M_WAY_POST ) {
            pmail->m_fparam = mail->m_fparam;
            pmail->m_lparam = mail->m_lparam;
        }

        pmail->m_data_d     = 0;
        pmail->m_next       = NULL;
        pmail->m_ret_type   = mail->m_ret_type;  /* mail ret type */
        pmail->m_atime      = __ticks;
        mail->m_validate    = false;             /* invalidate */

        /* if sync send mail and need reply ,then return right away */
        if ( pmail->m_type == M_WAY_SEND
            && pmail->m_ret_type == M_RET_PROCEED )
            return (0); 
        /* wake up send mail process if stucked */
        if ( TS_SENDING == (procs_ptr[ mail->m_src_pid ])->thread[ mail->m_src_tid ].state ) {
            (procs_ptr[ mail->m_src_pid ])->thread[ mail->m_src_tid ].state = TS_RUNNING;
            schedule ();
        }
        return (0);
    }
    /* none mails ,self stuck */
    __curprocs->thread[cur_tid].state = TS_RECVING ;
    schedule ();
    return (-1);
}

/* ONLY for send mail */
u32 sys_replymail ( MAIL *pmail )
{   
    i16 src_pid,src_tid,dest_pid,dest_tid;
    pmail = (MAIL*) ((u32)(pmail) + (__curprocs->pid * USER_SPACE));

    if ( NULL == pmail ) {
        printk ( "wrong mail ptr ! \n" );
        return (-1);
    }

    /* get src dest pids,tids */

    src_pid = pmail->m_src_pid,src_tid = pmail->m_src_tid;  /* from (pid,tid) */
    dest_pid = __curprocs->pid,dest_tid = cur_tid;
    /* before reply mail ,we need make sure dest repled mail 
     * is wait for this reply action,just chk the dest mail
     * dest_pid,dest_tid,etc .....( one to one chk )
     */

    /* ASYNC mail denied ! */
    if ( pmail->m_type != M_WAY_POST || pmail->m_ret_type != M_RET_PROCEED ) /* no need to reply */ 
        return (-1);
    return (-1);
}

