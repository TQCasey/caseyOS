#include <casey/types.h>
#include <casey/excptn.h>
#include <string.h>
#include <casey/kernel.h>
#include <casey/sched.h>

/* global val */
__u8 		GdtPtr[6];      	//Low 16 bits <-- limit,high 32 bits <-- base 
__u8        IdtPtr[6] = {0};
GATE        IDT[MAX_IDT_NUM];
DESCRIPTOR 	GDT[MAX_GDT_NUM];   //NEW GDT
TSS         __tss;

/* init GDTPtr */
static void InitGDTPtr ( void ) 
{
	__u32 *pGdtBase  = (__u32*)(GdtPtr+2); 	//dword [ptr + 2]
	__u16 *pGdtLimit = (__u16*)(GdtPtr+0); 	// word [ptr + 0]

	//copy old GDT to New GDT ...
	memcpy ( (void*)&GDT,				//Dest <-- dest (edi)
	 (void*)(*pGdtBase), 				//Base <-- dword ptr [GdtPtr + 2 ] --> Src (esi)
	 *pGdtLimit + 1 );				    //LImit<-- word  ptr [GdtPtr + 0 ] --> len (ecx)
	//copy complete!
	
	//change gdtptr
	*pGdtBase  = (__u32)&GDT;
	*pGdtLimit = (__u16)( MAX_GDT_NUM * sizeof(DESCRIPTOR) - 1 );	
}

/* init IDTPtr */
static void InitIDTPtr ( void ) 
{
	__u32 *pIdtBase  = (__u32*)(IdtPtr+2); 	//dword [ptr + 2]
	__u16 *pIdtLimit = (__u16*)(IdtPtr+0); 	// word [ptr + 0]
	
	//change idtptr
	*pIdtBase  = (__u32)&IDT;
	*pIdtLimit = (__u16)( MAX_IDT_NUM * sizeof(GATE) - 1);	
}


/* 8259A init */
static void Init8259A ( void )
{
	wrport (M_CTL	,0x11); 	    // need ICW4
	wrport (S_CTL	,0x11); 	    // need ICW4
	wrport (M_MASK	,INT_VECT_IR0); 
	wrport (S_MASK	,INT_VECT_IR8); 
	wrport (M_MASK	,0x04);		    //(ICW3)IR2   <-- Slaver
	wrport (S_MASK	,0x02);		    //(ICW3)Slaver--> IR2
	wrport (M_MASK	,0x01);		    //(ICW4)Model = 80x86,Normal EOI,Buf_mod=0 ,sequential model
	wrport (S_MASK	,0x01);		    //(ICW4)Model = 80x86,Normal EOI,Buf_mod=0 ,sequential model
	wrport (M_MASK	,0xFF);		    //(OCW1)Mask int
	wrport (S_MASK	,0xFF);		    //(OCW1)Mask int
}

/* init int descriptor */
static void InitIDTDesc ( __u8 vectno,		          //vect num
		                  __u8 desctype, 			  //desc type
		                  PRC PRC,		    	      //int PRC
		                  __u8  dpl )				  //DPL		    
{
	__u32 offset            = (__u32)PRC ;
	IDT[vectno].OffsetLow 	= offset & 0xFFFF;		  //mask high 16 bits
	IDT[vectno].Selector  	= SELECTOR_KERNEL_CS;	  //selector kernel 
	IDT[vectno].DCount    	= 0;	    			  //Not Used in int gate
	IDT[vectno].Attr	    = desctype | (dpl << 5);  //DPL (2 bits )
	IDT[vectno].OffsetHigh  = (offset >> 16) & 0xFFFF;//offset high 16 bits
}

extern void __do_syscall ( void );

#define set_trap_gate(vect_no,prc)\
    InitIDTDesc (vect_no,DA_386TGate,(PRC)prc,PL_KNRL );
#define set_intr_gate(vect_no,prc)\
    InitIDTDesc (vect_no,DA_386IGate,(PRC)prc,PL_KNRL );
#define set_sys_gate(vect_no,prc)\
    InitIDTDesc (vect_no,DA_386IGate,(PRC)prc,PL_USER );

/* init irqs && excptns */
static void InitIntPrc ( void ) 
{

	Init8259A ();

    set_intr_gate (INT_VECT_DE,__div_err); // int 0 
    set_intr_gate (INT_VECT_DB,__debug); // int 1
    set_intr_gate (INT_VECT_NMI,__nmi); // int 2
    set_intr_gate (INT_VECT_BP,__breakpt); // int 3
    set_intr_gate (INT_VECT_OF,__overflow); // int 4
    set_intr_gate (INT_VECT_BR,__bounds_chk); // int 5
    set_intr_gate (INT_VECT_UD,__inval_opcode); // int 6
    set_intr_gate (INT_VECT_NM,__copr_unavailable); // int 7
    set_intr_gate (INT_VECT_DF,__double_fault); // int 8
    set_intr_gate (INT_VECT_OVRR,__copr_seg_overrun); // int 9
    set_intr_gate (INT_VECT_TSS,__inval_tss); // int 10
    set_intr_gate (INT_VECT_NP,__seg_unpresent); // int 11
    set_intr_gate (INT_VECT_SS,__stack_exception); // int 12
    set_intr_gate (INT_VECT_GP,__general_protection); // int 13
    set_intr_gate (INT_VECT_PF,__page_fault); // int 14
    set_intr_gate (INT_VECT_MF,__copr_err); // int 16
}

/* for init irqs */
static void InitIRQPrc ( void )
{
	//8259 IntVect
	set_intr_gate (INT_VECT_IR0 + 0  ,__clk ); //  clk
	set_intr_gate (INT_VECT_IR0 + 1  ,__keyboard); //  keyboard
/*  set_intr_gate ( INT_VECT_IR0 + 2  ,DA_386IGate,handle_ints	        	    ,0x00 ); // int 0x22
	set_intr_gate ( INT_VECT_IR0 + 3  ,DA_386IGate,handle_ints	        	    ,0x00 ); // int 0x23
	set_intr_gate ( INT_VECT_IR0 + 4 	,DA_386IGate,handle_ints	        	,0x00 ); // int 0x24
	set_intr_gate ( INT_VECT_IR0 + 5 	,DA_386IGate,handle_ints	        	,0x00 ); // int 0x25
	set_intr_gate ( INT_VECT_IR0 + 6 	,DA_386IGate,handle_ints		        ,0x00 ); // int 0x26
	set_intr_gate ( INT_VECT_IR0 + 7 	,DA_386IGate,handle_ints		        ,0x00 ); // int 0x27
	set_intr_gate ( INT_VECT_IR8 + 0  ,DA_386IGate,handle_ints	        	    ,0x00 ); // int 0x28 
	set_intr_gate ( INT_VECT_IR8 + 1 	,DA_386IGate,handle_ints		        ,0x00 ); // int 0x29
	set_intr_gate ( INT_VECT_IR8 + 2 	,DA_386IGate,handle_ints	        	,0x00 ); // int 0x2A
	set_intr_gate ( INT_VECT_IR8 + 3 	,DA_386IGate,handle_ints		        ,0x00 ); // int 0x2B
	set_intr_gate ( INT_VECT_IR8 + 4 	,DA_386IGate,handle_ints	        	,0x00 ); // int 0x2C
	set_intr_gate ( INT_VECT_IR8 + 5 	,DA_386IGate,handle_ints	        	,0x00 ); // int 0x2D
*/	
    set_intr_gate ( INT_VECT_IR8 + 6,__hd); // hard disk 
//	set_intr_gate ( INT_VECT_IR8 + 7 	,DA_386IGate,handle_ints		        ,0x00 ); // int 0x2F
	//sys_calls &&  msg calls
	set_sys_gate (INT_VECT_SYS,__do_syscall); // int 0x80
}
/* init descriptor */
void InitDT ( DESCRIPTOR *pDesc,
	            __u32 base,
	            __u32 limit,
	            __u16 attri ) 
{
    pDesc->LimitLow  	=  limit & 0xFFFF;                      //  0 - 15  ( 16 bytes ) 
	pDesc->BaseLow   	=  base  & 0xFFFF;                      // 16 - 31  ( 16 bytes )
    pDesc->BaseMid      =  (base >> 16) & 0xFF;                 // 32 - 39  ( 8 bytes  ) 
    pDesc->Attr         =  attri & 0xFF;                        // 40 - 47  ( 8 bytes  )
    pDesc->LimitAttr    =  ((limit>>16) & 0x0F ) |              // 48 - 51  ( 4 bytes  )
                           ((attri>>8)  & 0xF0 ) ;              // 52 - 55  ( 4 bytes  )
	pDesc->BaseHigh 	=  (base>>24)   & 0xFF;                 // 56 - 63  ( 8 bytes  )
}


/* set descriptor */
void set_ldt ( __u32 nr_procs,__u8 nr_ldt,
               __u32 base,__u32 limit,__u16 attri ) 
{
    PROCESS *tmp = procs_ptr[nr_procs] ;
    InitDT ( &(tmp->ldts[nr_ldt%MAX_LDT_NUM]),base,0x10,attri ) ;
}

/* get phys addr(base) from dt seg */
inline __u32 seg2phys ( __u16 seg ) 
{
	DESCRIPTOR  *pDesc = (DESCRIPTOR*)&GDT[seg >> 3];	
	return (pDesc->BaseHigh << 24 |
		    pDesc->BaseMid  << 16 |
		    pDesc->BaseLow         );
}

/* init tss ,just use one tss */
static void InitTSS ( void ) 
{
	//fill tss
	memset ( &__tss,0,sizeof(__tss) );
	__tss.ss0 = SELECTOR_KERNEL_DS;
	__tss.iobase = sizeof(__tss);       /* all i/o is allowed  */
	
	//fill tss
	InitDT ( (DESCRIPTOR*)(&GDT[INDEX_TSS]),
		vir2phys ( seg2phys (SELECTOR_KERNEL_DS),&__tss),
		sizeof(__tss) - 1,
		DA_386TSS  );
}

/* add a table_obj in gdt_table ,and load tss*/
void InitProcsLdt ( PROCESS *procs_p  ) 
{
    //add a table obj in GDT_table 
    InitDT ((DESCRIPTOR*)(&GDT[procs_p->pid+INDEX_LDT_FIRST]),
        vir2phys ( seg2phys (SELECTOR_KERNEL_DS),procs_p->ldts),
        //get the procs table ltds entry addr
        MAX_LDT_NUM * sizeof(DESCRIPTOR) - 1,
        DA_LDT    );
} 


/* read just one byte from port */
unsigned char   rdport  ( unsigned short port )
{
    __byte tmp = 0x00;
    rdports (port,(const void*)&tmp,1);
    return (tmp);
}

/* write just one byte to port */
unsigned char   wrport  (unsigned short port,unsigned char val)
{
    return ( *((__byte*)wrports (port,(const void*)&val,1)) );
}


/* for kernel cstart entry point */
void __kernel_cstart ( void ) 
{ 
    InitGDTPtr ();
    InitIDTPtr ();
    InitIntPrc ();
    InitIRQPrc ();
    InitTSS    ();
}

