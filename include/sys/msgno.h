#ifndef __MSG_H__
#define __MSG_H__

#include <casey/types.h>

/* msg struct 
 * must be 32 bytes 
 */
#define         MDD_SEND        1
#define         MDD_RECV        2
typedef struct  tagMAIL{
    unsigned short      m_msg;          // umsg 
    unsigned char       m_level;        // mail level 
    unsigned char       m_locked;       // mail lock (false for unused)

    unsigned long       m_fparam;       // first param  
    unsigned long       m_lparam;       // last param 
    unsigned long       m_atime;        // arrive time

    short               m_src_pid;      // from which process 
    short               m_src_tid;      // from which thread         
    short               m_dest_pid;     // to which process 
    short               m_dest_tid;     // to which thread

    unsigned char       m_validate;     // is valid ?
    unsigned char       m_type;         // msg type 
    unsigned char       m_ret_type;     // ONLY validate to sendmail
    unsigned char       m_data_d;       // data stream direction

    struct  tagMAIL     *m_next;        // next send 
}MAIL;

/* each process has 4KB for mail box 
 *    0 - 1023  for sys_mail_box    32 mails
 * 1024 - 3071  for usr mail box    64 mails 
 * 3072 - 4095  specailly for sendmail slots
 */

#define NR_SYS_MAILS    31
#define NR_SND_MAILS    32
#define NR_USR_MAILS    64

typedef struct {
    MAIL    *head,*tail;
    i32     cnt;
}WAIT_Q;

typedef struct tagMAILBOX{
    i8              mb_sys_cnt;         /* sys mails cnt */
    i8              mb_usr_cnt;         /* usr mails cnt */
    i16             mb_snd_cnt;         /* snd mails cnt */
    WAIT_Q          post,send;
    i8              unused[4];
    MAIL            sys_m[NR_SYS_MAILS];/* sys mail slots */
    MAIL            usr_m[NR_USR_MAILS];/* usr mail slots */
    MAIL            snd_m[NR_SND_MAILS];/* snd mail slots */
}MAILBOX;


#define M_WAY_POST              0x00    /* post mail */
#define M_WAY_SEND              0x01    /* send mail */

#define M_RET_NOPROCEED         0x00    /* return right now */
#define M_RET_PROCEED           0x01    /* return till proceeded !*/
                        
#define MPL_SYS                 0x00    /* syslevel message  */
#define MPL_USR                 0x01    /* usr level message */


#define MSG_ANY                 0x01    /* any msg  */
#define MSG_SND                 0x02    /* send msg */
#define MSG_PST                 0x04    /* post msg */

/* FS message */
#define FSM_OK                  0x0101
#define FSM_READ                0x0102
#define FSM_WRITE               0x0103
#define FSM_EXEC                0x0104

/* TTY message */
#define TTYM_OK                 0x0201
#define TTYM_READ               0x0202

/* MM Messgae */
#define MMM_OK                  0x0301
#endif
