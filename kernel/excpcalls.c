#include <casey/types.h>
#include <casey/kernel.h>
#include <casey/sched.h>


extern i16 cur_tid;
static void div_err ( void ) 
{
    panic ( "divide error !\n" );
}

static void debug ( void ) 
{
    panic ( "debugging ..." );
}

static void nmi ( void ) 
{
    panic ( "non - mask - interrput... !\n" );
}

static void break_pt ( void ) 
{
    panic ( "breadk point !\n" );
}

static void over_flow ( void )
{
    panic ( "over flow !\n" );
}

static void bounds_chk ( void )
{
    panic ( "bounds chk cross !\n" );
}

static void invalidate_opcode ( void ) 
{
    panic ( "invalidate opretor code at process %d, thread %d \n"
            "cs  0x%0x eip 0x%0x eflags 0x%0x !\n",__curprocs->pid,cur_tid,
            __curprocs->regs.cs,__curprocs->regs.eip,__curprocs->regs.eflags );
}

static void corp_unpresent ( void ) 
{
    panic ( "corprocessor unpresent !\n" );
}

static void double_fault ( void ) 
{
    panic ( "double fault !\n" );
}

static void copr_seg_overrun ( void )
{
    panic ( "corprocessor seg fault !\n" );
}

static void invalidata_tss ( __u32 err_code )
{
    panic ( "invalidate tss !\n" );
}

static void seg_unpresent ( __u32 err_code )
{
    panic ( "segment not present !\n" );
}

static void stack_err ( __u32 err_code ) 
{
    panic ( "stack error !\n" );
}

static void general_protection ( __u32 err_code ) 
{
    panic ( "general protection at process %d, thread %d \n"
            "cs  0x%0x eip 0x%0x eflags 0x%0x !\n",__curprocs->pid,cur_tid,
            __curprocs->regs.cs,__curprocs->regs.eip,__curprocs->regs.eflags );
}

static void math_fault ( __u32 err_code ) 
{
    panic ( "math fault !\n" );
}

static void unknown_exceptn ( void )
{
    panic ( "unknown exception occured !\n" );
}

PRC __exceptn_prc[32];

#define PUT(x,prc)  __exceptn_prc[(x)] = prc;

void init_exceptn ( void ) 
{
    __u32 i = 0 ;
    for ( i = 0 ; i < 32 ;i ++ ) 
        __exceptn_prc[i] = unknown_exceptn;
    PUT(0,div_err);
    PUT(1,debug);
    PUT(2,nmi);
    PUT(3,break_pt);
    PUT(4,over_flow);
    PUT(5,bounds_chk);
    PUT(6,invalidate_opcode);
    PUT(7,corp_unpresent);
    PUT(8,double_fault);
    PUT(9,copr_seg_overrun);
    PUT(10,invalidata_tss);
    PUT(11,seg_unpresent);
    PUT(12,stack_err);
    PUT(13,general_protection);
    PUT(16,math_fault);
}
