/* -----------------------------------------------------------------------
 * for desc table decls
 * 2012 - 03 - 05 
 * By Casey 
 * ----------------------------------------------------------------------*/

#ifndef __DTS_H__
#define __DTS_H__


#include <casey/types.h>

#define MAX_IDT_NUM 	256
#define MAX_GDT_NUM		256         /* max_tasks = 64 */
#define MAX_LDT_NUM		2           /* no dummy desc */


//gate
typedef struct tagGATE{
	__u16   OffsetLow;                      //Offset Low
	__u16	Selector;                       //selector 
	__u8 	DCount;                         //used in call gate
	__u8	Attr;                           //P (1 bits) DPL (2 bits) DT (1 bits) TYPE (4 bits)
	__u16	OffsetHigh;                     //Offset High
}GATE,*LPGATE;

extern __u8 			IdtPtr[6];  	    //0 - 15 Limit,16 - 47 Base
extern GATE	    		IDT[MAX_IDT_NUM];   //New IDT

//GDT
typedef struct tagDESCRIPTOR{
	__u16	LimitLow;		                //limit Low 16 bits
	__u16 	BaseLow;                        //Base Low 16 bits
	__u8	BaseMid;                        //Base 	Mid 8 bits
	__u8 	Attr;                           //Attribute G (1 bits ),DPL(2bits),DT(3bits),TYPE(4 bits)
	__u8 	LimitAttr;                      //Limit high 4 bits ,Attribute Low 4 bits
	__u8 	BaseHigh;                       //Base High 8 bits
}DESCRIPTOR,*LPDESCRIPTOR;

extern __u8 			GdtPtr[6];      	//Low 16 bits <-- limit,high 32 bits <-- base 
extern DESCRIPTOR 		GDT[MAX_GDT_NUM];   //NEW GDT

//Data Seg Attribute
#define	DA_32			0x4000              // 32 Bits Seg				
#define	DA_LIMIT_4K		0x8000              // G = 1 (4KB)   			
#define	DA_DPL0			0x00                // DPL = 0				
#define	DA_DPL1			0x20                // DPL = 1				
#define	DA_DPL2			0x40                // DPL = 2				
#define	DA_DPL3			0x60                // DPL = 3				

//ALl p = 1 ( All Present )
#define	DA_DR			0x90                // Read Data Seg		
#define	DA_DRW			0x92                // Read + Write Data Seg		
#define	DA_DRWA			0x93                // Read + Write (Accessed)Data Seg	
#define	DA_C			0x98                // Executable Code Seg		
#define	DA_CR			0x9A                // Executable + Read Code Seg		
#define	DA_CCO			0x9C                // Executable Code Seg ('O' stand for the same )		
#define	DA_CCOR			0x9E                // Executable + Read Code Seg		

#define	DA_LDT			0x82                // Is LDT Seg				
#define	DA_TaskGate		0x85                // Is Task Gate				
#define	DA_386TSS		0x89                // Is 386 TSS				
#define	DA_386CGate		0x8C                // Is 386 Call Gate			
#define	DA_386IGate		0x8E                // Is 386 Int Gate			
#define	DA_386TGate		0x8F                // Is 386 Trap Gate	


//Selector Atrribute
#define	SA_RPL_MASK		0xFFFC
#define	SA_RPL0			0
#define	SA_RPL1			1
#define	SA_RPL2			2
#define	SA_RPL3			3

//TI
#define	SA_TI_MASK		0xFFFB
#define	SA_TIG			0
#define	SA_TIL			4

//PL
#define PL_OS			0x00
#define PL_KNRL         PL_OS
#define PL_TASK			0x01
#define PL_USER			0x03

//RPL 
#define RPL_OS			PL_OS
#define RPL_KNRL        PL_OS
#define RPL_TASK 		PL_TASK
#define RPL_USER		PL_USER

//In Loader.asm
#define	INDEX_DUMMY		0	
#define	INDEX_FLAT_C	1
#define INDEX_CS		INDEX_FLAT_C	
#define	INDEX_FLAT_RW	2	
#define INDEX_DS		INDEX_FLAT_RW
#define	INDEX_VIDEO		3	
#define INDEX_GS		INDEX_VIDEO
#define	INDEX_TSS		4
#define	INDEX_LDT_FIRST	5
#define INDEX_LDT_C     0
#define INDEX_LDT_RW    1

#define SEL(X) ((X)<<3)

//Selector Val
#define	SELECTOR_DUMMY		SEL(INDEX_DUMMY)		//0x00
#define	SELECTOR_FLAT_C		SEL(INDEX_FLAT_C)		//0x08	
#define	SELECTOR_FLAT_RW	SEL(INDEX_FLAT_RW)		//0x10	
#define	SELECTOR_VIDEO		SEL(INDEX_VIDEO)+SA_RPL3//RPL == 3
#define	SELECTOR_TSS		SEL(INDEX_TSS)			// TSS.
#define SELECTOR_LDT		SEL(INDEX_LDT)

#define	SELECTOR_KERNEL_CS	SELECTOR_FLAT_C
#define	SELECTOR_KERNEL_DS	SELECTOR_FLAT_RW
#define	SELECTOR_KERNEL_GS	SELECTOR_VIDEO



//0x00 - 0x1F   exceptions ints
#define	INT_VECT_DE		0x00	// div err 
#define	INT_VECT_DB		0x01	// single step
#define	INT_VECT_NMI	0x02	// nmi
#define	INT_VECT_BP		0x03	// break point
#define	INT_VECT_OF		0x04	// over flow
#define	INT_VECT_BR		0x05	// bounds chk
#define	INT_VECT_UD		0x06	// invalid opt
#define	INT_VECT_NM		0x07	// device unavaliable
#define	INT_VECT_DF		0x08	// double fault
#define	INT_VECT_OVRR	0x09	// coproc seg over run ( NOT USED AFTER I386)
#define	INT_VECT_TSS	0x0A	// invalid tss
#define	INT_VECT_NP		0x0B	// seg unpresent
#define	INT_VECT_SS		0x0C	// stack error
#define	INT_VECT_GP		0x0D	// #GP error
#define	INT_VECT_PF		0x0E	// page fault
#define	INT_VECT_MF		0x10	// math fault

//Int Vectors Def
#define  INT_VECT_IR0	0x20    //Master Int Demo Entry
#define  INT_VECT_IR8  	0x28    //Slaver Int Demo Entry

//0x20 - 0x2F 8259A ints
#define  IRQ_CLK        0x00    //master clk
#define  IRQ_KBD        0x01    //key board
#define  IRQ_CASCADE    0x02    //cascade
#define  IRQ_HD         0x0E    //hard disk
//Port Name Def
#define  M_CTL	    	0x20	//master <-- OCW1
#define  S_CTL		    0xA0	//slaver <-- OCW1
#define  M_MASK 		0x21	//master <-- IMR
#define  S_MASK 		0xA1	//slaver <-- IMR

//Int Const
#define  EOI 			0x20

//0x80 - 0xFF for SysCall
#define INT_VECT_SYS    0x80    //sys_call server int
#define INT_VECT_MSG    0x81    //msg serve int

#define vir2phys(base,off)  \
    ((__u32)((__u32)base+(__u32)off))

extern inline __u32 seg2phys ( __u16 seg ) ;
extern void __kernel_cstart ( void ) ;
extern void set_ldt ( __u32 nr_procs,__u8 nr_ldt,
                    __u32 base,__u32 limit,__u16 attri ) ;
/* init descriptor */
extern void InitDT ( DESCRIPTOR *pDesc,
	                __u32 base,
	                __u32 limit,
	                __u16 attri ); 
#endif
