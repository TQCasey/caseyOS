#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/msgno.h>
#include <casey/sched.h>


char *_gets ( char *buf,int len ) 
{
    /*
    MAIL m;

    m.m_level = MPL_SYS;
    m.m_ret_type = M_RET_PROCEED;
    m.m_dest_pid = TTY;
    m.m_dest_tid = 2;
    m.m_date_d   = MDD_RECV;
    m.umsg = TTYM_READ;
    m.pkg[0] = (unsigned long)buf;
    m.pkg[1] = (unsigned long)len;

    sendm (&m);
    */
    return (buf);
}

char *get_data (short pid,short tid,char *buf,unsigned long len )
{
    MAIL m;

    /*
    m.m_level = MPL_SYS;
    m.m_ret_type = M_RET_PROCEED;
    m.m_dest_pid = pid;
    m.m_dest_tid = tid;
    m.m_date_d   = MDD_RECV;
    m.umsg = M_GET_MEM_BLK;
    m.pkg[0] = (unsigned long)buf;
    m.pkg[1] = (unsigned long)len;

    sendm (&m);
    */
    return (buf);
}

int exec (int argc,char **argv ,unsigned long size)
{ 
    MAIL m;

    /*
    m.m_level = MPL_SYS;
    m.m_ret_type = M_RET_PROCEED;
    m.m_dest_pid = FS;
    m.m_dest_tid = 0;
    m.m_date_d   = MDD_SEND;
    m.umsg = FSM_EXEC;
    m.pkg[0] = (unsigned long)argv;
    m.pkg[1] = (unsigned long)size;
    m.pkg[2] = argc;

    sendm (&m);

    */
    return (1);
}

