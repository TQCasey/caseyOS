/* ----------------------------------------------------------------------------
 * for kernel funcs decl 
 * 2012 - 3 -14  
 * By Casey 
 * ---------------------------------------------------------------------------*/
#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <casey/types.h>

/* kernel space */
#define MAX_ENTER_TIMES     8
#define TERM_BUFFER         0x9D000
#define TERM_BUF_LEN        0x500
#define K_VGA_START         0xB8000

/* for kernel && tasks rockect */
#define ROCKS               (3<<20)
#define FS_ROCK             (ROCKS)
#define TERM_ROCK           (FS_ROCK+0x1000)
#define MM_ROCK             (TERM_ROCK+0x1000)

#define sti()           __asm__("sti")
#define nop()           __asm__("nop")
#define cli()           __asm__("cli")

extern int  kputs ( const char* str ); 
extern int  kputch ( const char ch ) ;
extern inline int ksetpt ( __u32 posx,__u32 posy ); 
extern inline int kgetpt ( __u32* posx,__u32* posy ); 
extern inline __byte kgetrgb ( void ); 
extern inline __byte ksetrgb ( __byte crgb );
extern int    printk ( const char* fmt ,... ); 
extern int    panic ( const char *fmt,... ) ;

/* this is the decl for kernel module */
extern void   *rdports(short port,const char *dest,unsigned long  size);
extern unsigned long  wrports(short port,const char *src ,unsigned long  size);
extern unsigned char wrport ( unsigned short port,unsigned char val );
extern unsigned char rdport ( unsigned short port );

/* irq prcs opts */
#define MAX_IRQ_NUM     0x20
extern PRC __irq_prc[MAX_IRQ_NUM];
extern inline void rid_irq_prc ( __u32 irq_no ); 
extern void enable_irq ( __byte irq_num,bool fval ) ; 
extern inline void set_irq_prc ( __u32 irq_no,PRC prc ); 

extern __u32   MAPED_MEM;                       // mem for mapping ...
extern __u32   NR_PAGES;                        // mem pages 
extern __u8    *page_cnt;                       // pages cnt 
extern __u32   __mem_size;  
#define LOW_MEM      (4<<20)                    // 4MB for kernel space 

extern __u32 find_empty_page ( void ) ;

/* load mem to nr_procs process space */
extern void load_elf ( __u32 old_entry_base,short nr_procs ) ;

#define MAX_SYS_CALL    64
extern void *__sys_call [MAX_SYS_CALL];
extern unsigned long  NR_SYSCALL;

/* funcs prototypes here */
extern void __nop__ (void);

// for irqs decl 
extern void __clk ( void ) ;            // 0
extern void __keyboard ( void ) ;       // 1
extern void __hd ( void ) ;             // 14 

#define set_sys_call(sys_no,prc)    \
    __sys_call[sys_no]=prc;
#define rid_sys_call(sys_no)        \
    __sys_call[sys_no]=__nop__;

#endif
