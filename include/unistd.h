#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <sys/msgno.h>

/* msg syscalls */
#define __NR_postmail           63
#define __NR_sendmail           62
#define __NR_pickmail           61
#define __NR_replymail          60


/* sys calls */
#define __NR_get_tks            0
#define __NR_startup            1
#define __NR_exit               2
#define __NR_kill               3
#define __NR_tty_write          4
#define __NR_tty_read           5
#define __NR_pause              6
#define __NR_add_thread         7
#define __NR_end_thread         8
#define __NR_get_pinfo          9
#define __NR_wait_hd_intr       10
#define __NR_badness            11
#define __NR_block              12
#define __NR_unblock            13
#define __NR_waitpid            14

/* the syscalls argc == 0 */
#define sys_call_0(ret_type,func_name)   \
ret_type func_name(void) \
{\
    unsigned long __res;\
    __asm__ ("int  $0x80"\
            :"=a"(__res)\
            :"a"(__NR_##func_name));\
    return (__res);\
}

/* the syscall argc == 1 */
#define sys_call_1(ret_type,func_name,atype,a)   \
ret_type func_name(atype a) \
{\
    unsigned long __res;\
    __asm__ ("int  $0x80"\
            :"=a"(__res)\
            :"a"(__NR_##func_name),"b"(a));\
    return (__res);\
}

/* the syscall argc == 2 */
#define sys_call_2(ret_type,func_name,atype,a,btype,b)   \
ret_type func_name(atype a,btype b) \
{\
    unsigned long __res;\
    __asm__ ("int  $0x80"\
            :"=a"(__res)\
            :"a"(__NR_##func_name),"b"(a),"d"(b));\
    return (__res);\
}


/* the syscall argc == 3 */
#define sys_call_3(ret_type,func_name,atype,a,btype,b,ctype,c)   \
ret_type func_name(atype a,btype b,ctype c ) \
{\
    unsigned long __res;\
    __asm__ ("int  $0x80"\
            :"=a"(__res)\
            :"a"(__NR_##func_name),"b"(a),"d"(b),"c"(c));\
    return (__res);\
}

extern unsigned long    get_tks ( void );
extern unsigned long    startup ( void );
extern unsigned long    exit ( unsigned long exitcode );
extern unsigned long    kill (short pid);
extern unsigned long    tty_write ( const char *buf,unsigned long len ) ;
extern unsigned long    tty_read  ( char *buf,unsigned long size );
extern unsigned long    pause ( void );
extern          int     add_thread ( void* entry ,unsigned char state);
extern          int     end_thread ( short t_id );
extern unsigned long    get_pinfo ( unsigned long *tasks_nr ) ;
extern unsigned long    wait_hd_intr ( void );
extern unsigned long    block (short tid );
extern unsigned long    unblock (short tid );
extern unsigned long    waitpid (short pid,unsigned char *state);

extern void             delay_ms ( int ms );
extern void             delay_sc ( int sec );
extern void             crash ( const char *fmt,... ); 
extern int              getm ( MAIL *m,unsigned char flags ); 
extern int              postm ( MAIL *m ) ;
extern int              sendm ( MAIL *m ) ;
#define end()   end_thread(-1)

#endif
