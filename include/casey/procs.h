/*--------------------------------------------------------
* for procs decl | task 
* 2012 - 3 - 15
* By Casey
*--------------------------------------------------------*/
#ifndef __PROCS_H__
#define __PROCS_H__

#include <casey/types.h>
#include <sys/types.h>
#include <casey/dtcntl.h>   /* DESCRIPTOR declare in this header file */

//stack frame
typedef struct tagSTACK_FRAME{
	u32 	gs;
	u32 	fs;
	u32		es;
	u32		ds;
	u32		edi;
	u32		esi;
	u32		ebp;
	u32		kernel_esp;     // not used 
	u32		ebx;
	u32		edx;
	u32		ecx;
	u32		eax;
	u32		ret_addr;
	u32 	eip;            // user eip     (from caller)
	u32 	cs;             // user cs      (from caller)
	u32		eflags;         // user eflags  (from caller)
	u32		esp;            // user esp     (from caller)
	u32		ss;             // user ss      (from caller)
}STACK_FRAME;


#define MAX_THREAD      32

typedef struct {
    i16  from_pid,from_tid;
    i16  send_to_pid,send_to_tid;
}SEND_Q;

typedef struct { 
    short pid,tid; 
} PID;

typedef struct {
    u32       edi;
    u32       esi;
    u32       ebp;
    u32       kernel_esp;
    u32       ebx;
    u32       edx;
    u32       ecx;
    u32       eax;
    u32       ret_addr;
    u32       eip;
    u32       cs;
    u32       eflags;
    u32       esp;
}THREAD_REGS;


/* thread frame */
typedef struct tagTHREAD{
    u32       edi;
    u32       esi;
    u32       ebp;
    u32       kernel_esp;
    u32       ebx;
    u32       edx;
    u32       ecx;
    u32       eax;
    u32       ret_addr;
    u32       eip;
    u32       cs;
    u32       eflags;
    u32       esp;

    /* below this will be specfied in __sys_calls later */
    u8        soft_intr;                        /* soft ware interrupt flag */
    u32       t_ret_val;                        /* scheduled thread return value */
    i16       t_id;                             /* current thread id */
    i8        curtks,pl;                        /* current ticks ,priority */
    u8        state;                            /* thread state */
    PID       father;                           /* father pid,tid */
    SEND_Q    send_q;                           /* send que */
    u8        taken;                            /* token flag */
}THREAD;    

/* process struct */
typedef struct tagPROCESS{
    /* remember to modify asm/asmconst.inc if wanna to chage it */
	STACK_FRAME 	regs;                       // regs              
	u16		        ldtsel;		            	// Ldt Selector     
	DESCRIPTOR	    ldts[MAX_LDT_NUM];			// Ldt s
    u8              soft_intr_flag;             // soft interrupt flag
    i16             pid;                        // current process id
    THREAD          thread[MAX_THREAD];         // threads max nr
    u8              thread_nr;                  // current thread nr
    u8              uid,gid,euid;
    u32             m_root_inode,m_pwd_inode;
    i16             tty;                        // tty nr 
    void*           mail_box;                   // mail box page addr   
}PROCESS;

/* task segment struct */
typedef struct tagTSS{  
	u32 		link;			//back link
	u32 		esp0;			
	u32 		ss0;			// level 0 stack
	u32 		esp1;
	u32 		ss1;			// level 1 stack
	u32 		esp2;
	u32 		ss2;			// level 2 stack 
	u32 		cr3;			// CR3
	u32 		eip;			// eip
	u32 		eflags;
	u32 		eax;
	u32 		ecx;
	u32 		edx;
	u32 		ebx;
	u32 		esp;
	u32 		ebp;
	u32 		esi;
	u32 		edi;
	u32 		es;
	u32 		cs;
	u32 		ss;
	u32 		ds;
	u32 		fs;
	u32 		gs;
	u32 		ldt;
	u16		    trap;
	u16		    iobase;			//I/O Base 
}TSS,*LPTSS;

#endif
