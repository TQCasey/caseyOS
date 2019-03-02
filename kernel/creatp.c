#include <casey/types.h>
#include <casey/procs.h>
#include <casey/kernel.h>
#include <casey/mm.h>
#include <casey/sched.h>
#include <casey/dtcntl.h>
#include <string.h>

/* find a place for new process from procs_table */
static 
i16 find_free_procs ( void ) 
{
    i16 nr_procs = 0 ;

    for ( nr_procs = 0 ; nr_procs < MAX_PROCS ; nr_procs ++ ) {
        if ( !procs_ptr[nr_procs] ) 
            return (nr_procs);
    }
    /* return 0 means no Available procs_ptr */
    return (0);
}

/* cpy_procs ,just like linux 0.11 each process has 64MB space 
 * 4GB = 64 * 64MB ,so max_procs = 64 !!!!!!!!!!!!!!!!!!!!!!!*/

extern void InitProcsLdt ( PROCESS *procs  );
extern i16 cur_tid;


/* create process  assume nr_procs is okay !*/
static 
int mk_proc_obj(i16         nr_procs,
                char        prio,                 /* tks */
                u8          pl,                   /* privilage level */
                u8          rpl,                  /* rq privalage level */
                u32         eflags )              /* eflags */
{
    /* we use memcpy ,'cause the current procs is in KERNEL mem (<4M) 
     * the last 32KB for the process stack,the the 2nd last 32KB for 
     * mail box ,each process has a mail box,hope it works !!!!!!!!
     */
    u32 code_start = 0,stack_start = USER_SPACE;

    /* ONLY kernel can knows the PCB info */
    PROCESS *pcb = (PROCESS*)find_empty_page ();
    if ( !pcb ) return (-1);
    procs_ptr[nr_procs] = pcb;

    /* do pcb->PCB */
    pcb->pid                = nr_procs;
    /* set father pid,tid */
    pcb->thread[cur_tid].father.pid = __curprocs->pid;
    pcb->thread[cur_tid].father.tid = cur_tid;
    pcb->mail_box           = (void*)find_empty_page ();        /* mail box page addr */
    /* code start & end limit settings */
    u32 code_base = 0,data_base = 0,code_limit = 0,data_limit = 0;
    code_base   = data_base = (USER_SPACE * nr_procs);// 4KB alignment
    code_limit  = data_limit= 0x4000000>>12 ;

    /* ldt pcb->...*/
    pcb->ldtsel             = SEL(INDEX_LDT_FIRST+nr_procs);
    InitDT ( &(pcb->ldts[0]),code_base,code_limit,DA_C|DA_32|(pl<<5)|DA_LIMIT_4K);
    InitDT ( &(pcb->ldts[1]),code_base,code_limit,DA_DRW|DA_32|(pl<<5)|DA_LIMIT_4K);

    /* target eip setup now*/
    pcb->regs.eip = code_start ;
    pcb->regs.esp = stack_start;

    /* segment regs init */
    pcb->regs.cs            = (SEL(INDEX_LDT_C) & SA_RPL_MASK & SA_TI_MASK)| SA_TIL| rpl;
    pcb->regs.ds            = 
    pcb->regs.es            = 
    pcb->regs.fs            = 
    pcb->regs.ss            = (((SEL(INDEX_LDT_RW)) & SA_RPL_MASK) & SA_TI_MASK)| SA_TIL|rpl;
    pcb->regs.gs            = ((((SELECTOR_KERNEL_GS) & SA_RPL_MASK)) | rpl) ;

    /* main thread init... */
    THREAD *p = &(pcb->thread[0]);
    p->cs                   = (SEL(INDEX_LDT_C) & SA_RPL_MASK & SA_TI_MASK)| SA_TIL| rpl;
    p->curtks               = pcb->thread[0].pl = prio;
    p->eip                  = code_start;
    p->esp                  = stack_start;
    p->eflags               = eflags;
    p->state                = TS_RUNNING;
    p->t_id                 = 0;
    p->send_q.send_to_pid   = p->send_q.send_to_tid = -1;   /* no send pid,tid */
    p->send_q.from_pid      = p->send_q.from_tid    = -1;   /* no src */
    pcb->thread_nr          = 1;
    InitProcsLdt (pcb);

    if ( nr_procs < MAX_TASKS ) 
        NR_TASKS ++;
    else
        NR_USR_PROCS ++;
    return (0);         /* ok for cpy */
}

/* copy mem from __current process */
extern u32 map_page_to_phys ( u32 phys_addr,u32 page_addr ) ;

/* shell mem prepare */
int prepare_shell_mem ( i16 nr_procs ) 
{
    load_elf (0x0d000,nr_procs);
    return (1);
}
/* fs mem prepare */
static 
int prepare_fs_mem ( i16 nr_procs ) 
{
    /* load fs bin to 0x4000000 */
    load_elf (0x5d000,nr_procs);

    u32  i = 0 ,pages = 256 ,k_fsbuf = 2<<20,
          u_fsbuf = nr_procs * USER_SPACE + FSBUF;
    /* map 2M - 3M to fs ((64M-3M) - (64M - 2M)) zone */
    for ( i = 0 ; i < pages ; i ++ ) 
    {
        map_page_to_phys (k_fsbuf,u_fsbuf);
        k_fsbuf += 0x1000;
        u_fsbuf += 0x1000;
    }
    /* map fs rockect */
    map_page_to_phys (FS_ROCK,
            nr_procs * USER_SPACE + ROCKECT);

    return (1);
}

/* tty mem prepare */
static 
int prepare_tty_mem ( i16 nr_procs ) 
{
    /* load tty bin to 0x4000000 * 2 */
    load_elf (0x3d000,nr_procs);
    /* map terminal process mem (STDIN) (4KB)*/
    map_page_to_phys (TERM_BUFFER,
            (nr_procs * USER_SPACE + STDIN_BUF) );
    /* map terminal process mem (STDOUT ) (32KB) */
    u32 i = 0 ,k_vga = K_VGA_START,u_stdout = STDOUT_BUF; 
    for ( i = 0 ; i < 8 ; i ++ ) 
    {
        map_page_to_phys (k_vga,
                (nr_procs * USER_SPACE + u_stdout));
        k_vga += 0x1000;
        u_stdout += 0x1000;
    }
    /* map rocket buffer */
    map_page_to_phys (TERM_ROCK,
            nr_procs * USER_SPACE + ROCKECT);

    return (1);
}
/* mm mem prepare */
static 
int prepare_mm_mem ( i16 nr_procs ) 
{
    /* load mm bin to 0x4000000 * 3 */
    load_elf (0x1d000,nr_procs);
    return (1);
}

/* usr mem prepare */
static 
int prepare_usr_mem ( i16 nr_procs ) 
{
    //memcpy (nr_procs * USER_SPACE,show,0x1000);
    return (1);
}

static 
int prepare_mem ( i16 nr_procs ) 
{
    switch (nr_procs)
    {
        case 1:     return prepare_shell_mem (nr_procs);
        case 2:     return prepare_fs_mem (nr_procs);
        case 3:     return prepare_tty_mem (nr_procs);
        case 4:     return prepare_mm_mem (nr_procs);
        default:    panic ( "more than %d task startup !\n",MAX_TASKS );
    }
    return (1) ;
}

/* well....,fs can be changed as a undependent part (as to kernel),we need to compile 
 * the fs module alone so that we can relocate entry in process space,actually fs is 
 * a process too.to appoarch that aim ,we need to def a func here which can recognize
 * the elf fmt of fs moulde,(hope it works !,orelse I will ....)
 * NOTE:  all _start is 0x00000000 (don't forget that)
 * 2012 - 4 - 13 by casey 
 */

/* ONLY kernel can do it ,this is comm interface of elf loader*/
void load_elf ( u32 old_entry_base ,i16 nr_procs ) 
{
    u16 e_phnum = getw (old_entry_base+0x2c);          
    u32 e_phoff = getdw (old_entry_base+0x1c) + old_entry_base;
    u32 new_entry_base,size,src = 0;

    if ( old_entry_base > __mem_size || (nr_procs > (NR_TASKS + NR_USR_PROCS)) ) 
        panic ( "bad relocation ");
    if ( !e_phnum || !e_phoff ) 
        panic ( "bad program headers && bad ph offset ");

    u16 i = 0 ; 
    for ( i = 0 ; i < e_phnum ; i ++ ,e_phoff += 0x20 )
    {
        /* if e_type == PT_NULL ,to next ph */
        if ( !getw(e_phoff) ) continue ;
        size = getdw (e_phoff + 0x10);
        src = getdw (e_phoff + 0x04) + old_entry_base;
        new_entry_base = getdw (e_phoff + 0x08) + nr_procs * USER_SPACE;                        
        memcpy ((void*)new_entry_base,(void*)src,size);
        new_entry_base += size;
    }
}

/* create a usr process  */
i32 sys_creatp (int argc,char **argv,char **envp) 
{
    i16 nr_procs = find_free_procs ();
    if ( !nr_procs ){
        printk ( "more than 64 process is created !\n" );
        return (-1);
    }
    i16 pid = __curprocs->pid;
    char (*p)[30];
    p = (char**)((u32)argv + USER_SPACE * pid);

    while ( argc -- ) kputs (*p ++);


    return (nr_procs);
}


u32 sys_startup (void) 
{
    i16 nr_procs = find_free_procs ();

    if ( !nr_procs ){
        printk ( "more than 64 process is created !\n" );
        return (-1);
    }
    if (nr_procs >= MAX_TASKS)  return (0);

    int ret ;
    if ( (ret = mk_proc_obj (nr_procs,4,PL_TASK,RPL_TASK,0x1202)) < 0 ) {
        printk ( "make proc object failed !\n" );
        return (0);
    }

    prepare_mem (nr_procs);
    return (nr_procs);
}
