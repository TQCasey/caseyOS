#include <sys/msgno.h>
#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>
#include <casey/mm.h>
#include <string.h>

extern u32 __ticks;
extern i16 cur_tid;

/* to clean up invlidate mails */
void clean_up ( MAILBOX *mailbox ) 
{
}

/* to find a mail slot of (m_type,m_level) which belong to 
 * dest_pid ,dest_tid 
 */

MAIL* find_empty_mail_slot ( u8 m_type,
        i16    dest_pid,            /* dest pid */
        i16    dest_tid,            /* dest tid */
        u8     m_level,             /* level */
        u8     **count ) 
{
    MAILBOX *mailbox = (MAILBOX*)(procs_ptr[dest_pid]->mail_box);
    MAIL *mail;
    int i,mail_nr;

    if ( !m_type ){  /* post mail */
        if ( m_level == MPL_SYS ){  /* sys mail */
            mail = mailbox->sys_m;     
            *count = (u8*)&(mailbox->mb_sys_cnt);
            mail_nr = NR_SYS_MAILS;   
        }
        else {                      /* usr mail */
            mail = mailbox->usr_m;     
            *count = (u8*)&(mailbox->mb_usr_cnt);
            mail_nr = NR_USR_MAILS;   
        }
        /* for post mail slots */
        for ( i = 0 ; i < mail_nr ; i ++ ) 
            if ( !mail[i].m_locked && !mail[i].m_validate ) /* invlalidate && open */
                return (mail + i);
    }
    else {           /* send mail */
        /* send mail has 32 send_mail_slots ,each thread has the ONLY
         * one which index by the tid !!!!
         */
        mail = (MAIL*)(&mailbox->snd_m[dest_tid]);
        *count = (u8*)&(mailbox->mb_snd_cnt);
        return (mail);
    }
    return (NULL)   ;/* no enough mail slot  */
}

/* find mails from mailbox */
MAIL* find_mb_mail (i16 dest_pid,i16 dest_tid,
        u16 m_msg,u8 **count,u8 flags ) 
{
    MAILBOX *mailbox = (MAILBOX*)(procs_ptr[dest_pid]->mail_box);
    MAIL *mail;
    int i,mail_nr,level = 0; 
    char cnt  ;

    /* find mail from mailbox ,the order as follows 
     * send (sys->usr) --> post (sys->usr) 
     */

    if ( MSG_SND == (MSG_SND & flags) || flags == MSG_ANY ) {   /* send mail */

    }

    if ( MSG_PST == (MSG_PST & flags) || flags == MSG_ANY ) {   /* post mail */
        for ( ; level < 2 ; level ++ ) 
        {
            if (!level) {
                mail    = mailbox->sys_m;     
                cnt     = mailbox->mb_sys_cnt;
                *count  = &(mailbox->mb_sys_cnt);
                mail_nr = NR_SYS_MAILS;   
            }else {
                mail    = mailbox->usr_m;     
                cnt     = mailbox->mb_usr_cnt;
                *count  = &(mailbox->mb_usr_cnt);
                mail_nr = NR_USR_MAILS;  
            }

            for ( i = 0 ; i < mail_nr && cnt > 0  ; i ++   ) 
            {   
                if ( mail[i].m_locked && mail[i].m_validate ) {/* mails */
                    cnt --;
                    /* my mail */
                    if ( mail[i].m_dest_pid == __curprocs->pid 
                        && mail[i].m_dest_tid == cur_tid  ) 
                    {
                        /* any message */
                        if ((flags & MSG_ANY) == MSG_ANY) 
                            return (mail+i);
                        else if (mail[i].m_msg == m_msg ) /* specified */
                            return (mail+i);
                    }
                }
            }
        }
    }
    return (NULL)   ;/* not found  */
}
