#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/msgno.h>
#include <casey/procs.h>

#define HZ  100
/* postmail */
static sys_call_1(unsigned long,postmail,MAIL*,pmail)
static sys_call_1(unsigned long,sendmail,MAIL*,pmail)
static sys_call_2(unsigned long,pickmail,MAIL*,pmail,unsigned char,flags)
static sys_call_1(unsigned long,replymail,MAIL*,pmail)

/* this is for sys call */
sys_call_0(unsigned long,get_tks)                                       /* get tks */
sys_call_0(unsigned long,startup)                                       /* start up */
sys_call_1(unsigned long,exit,unsigned long,exitcode)                   /* exit (exitcode) */         
sys_call_1(unsigned long,kill,short ,pid)                                /* kill process  */
sys_call_2(unsigned long,tty_write,const char*,buf,unsigned long ,len)  /* tty_write (buf,len) */
sys_call_2(unsigned long,tty_read,char*,buf,unsigned long ,len)   /* tty_read (buf,len)  */
sys_call_0(unsigned long,pause)
sys_call_2(int,add_thread,void* ,entry,unsigned char ,state)
sys_call_1(int,end_thread,short,tid)
sys_call_1(unsigned long,get_pinfo,unsigned long*,tasks_nr)
sys_call_0(unsigned long,wait_hd_intr)
static sys_call_0(unsigned long,badness)
sys_call_1(unsigned long,block,short,tid)
sys_call_1(unsigned long,unblock,short,tid)
sys_call_2(unsigned long,waitpid,short,pid,unsigned char*,state)


void delay_ms ( int ms )     
{
    unsigned long t = get_tks () ,alarm = 0;
    while ( (alarm = ((get_tks() - t) * 1000 / HZ)) < ms );
}

void delay_sc ( int sec ) 
{      
    unsigned long  t = get_tks ();
    while ( (get_tks() - t) / HZ < sec ) ;    
}

void crash ( const char *fmt,... ) 
{
    va_list va_p = (void*)0;
    va_start(va_p,fmt);
    char buf[256 + 128] = {'\n'};

    unsigned long len = vsprintf (buf+1,fmt,va_p);
    tty_write (buf,len);

    va_end (va_p);
    badness ();

    while ( 1 ) ;
}

int getm ( MAIL *m,unsigned char flags ) 
{
    int ret = 0;
    while ( (ret = pickmail (m,flags)) < 0 ) ;
    return (ret);
}

int postm ( MAIL *m ) 
{
    int ret  = postmail (m);
    return (ret);
}
