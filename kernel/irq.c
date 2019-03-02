#include <casey/types.h>
#include <casey/dtcntl.h>
#include <casey/kernel.h>

PRC __irq_prc[MAX_IRQ_NUM];

/* install irq prc */
inline void set_irq_prc ( __u32 irq_no,PRC prc ) 
{   
    enable_irq (irq_no,false);                              //first we disable the irq
    __irq_prc [irq_no] = prc  ;                             //after replacing the irq_prc
    enable_irq (irq_no,true);                               //we enable this irq
} 

/* do nothing */
void __nop__ ( void ) 
{
    return;
}

/* true == val for enable ,false for disable */
void enable_irq ( __byte irq_num,bool fval )  
{
        unsigned short port = (irq_num <8) ? (M_MASK) : (S_MASK) ;
        __byte val = rdport ( port );
        
        wrport (port,
                (fval != true) ? 
                        (val |  (1<< (irq_num%8))) :            //if true ,disable it 
                        (val & ~(1<<(irq_num%8))) );            //or not ,enable it
}

/* uninstall irq prc */
inline void rid_irq_prc ( __u32 irq_no ) 
{
    enable_irq (irq_no,false);
    __irq_prc [irq_no] = (PRC)__nop__;
    enable_irq (irq_no,true);
}
