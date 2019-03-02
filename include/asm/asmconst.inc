; ============================================================
; asmconst.inc
; ============================================================
%ifndef __SCONST_INC__
%define __SCONST_INC__
;procs_table

P_STACKBASE	             equ	0
GSREG		             equ	P_STACKBASE                         ; gs 
FSREG		             equ	GSREG		        + 4             ; fs
ESREG		             equ	FSREG		        + 4             ; es
DSREG		             equ	ESREG		        + 4             ; ds
EDIREG		             equ	DSREG		        + 4             ; edi
ESIREG		             equ	EDIREG	        	+ 4             ; esi
EBPREG		             equ	ESIREG	        	+ 4             ; ebp
KERNELESPREG	         equ	EBPREG	        	+ 4             ; kernel_esp
EBXREG	        	     equ	KERNELESPREG	    + 4             ; ebx
EDXREG		             equ	EBXREG      		+ 4             ; edx
ECXREG		             equ	EDXREG	        	+ 4             ; ecx
EAXREG	        	     equ	ECXREG	        	+ 4             ; eax
RETADR	            	 equ	EAXREG	        	+ 4             ; ret_addr
EIPREG		             equ	RETADR      		+ 4             ; eip
CSREG		             equ	EIPREG	        	+ 4             ; cs
EFLAGSREG                equ	CSREG		        + 4             ; eflags
ESPREG                   equ	EFLAGSREG	        + 4             ; esp
SSREG                    equ	ESPREG		        + 4             ; ss
P_STACKTOP               equ	SSREG		        + 4             ; 
P_LDT_SEL                equ	P_STACKTOP                          ; ldtsel
P_LDT                    equ	P_LDT_SEL	        + 4             ; &ltds[0]  

; tss 
TSS3_S_SP0	equ	4


SELECTOR_CS		        equ 	0x08
SELECTOR_DS		        equ 	0x10
SELECTOR_GS		        equ 	0x18
SELECTOR_TSS	    	equ 	0x20
SELECTOR_LDT	    	equ 	0x28
KERNEL_SELECTOR_CS	    equ     0x08


M_CTL			        equ		0x20	
S_CTL			        equ		0xA0	
M_MASK 		        	equ		0x21	
S_MASK 			        equ		0xA1	
EOI 			        equ 	0x20

TRUE                    equ     0x01
FALSE                   equ     0x00

true                    equ     0x01
false                   equ     0x00

MAX_IRQ_NUM             equ     0x80           ; 0x80 must equ the MAX_IRQ_NUM in ints.h
MAX_SYS_NUM             equ     0x80           ; 0x80 syscall equ MAX_SYSCALL_NUM in ints

; syscall_irq 
INT_VECT_SYS            equ     0x80           ; equ ints.h
; msg _sys_call 
INT_VECT_MSG            equ     0x81           ; equ ints.h

HARD_INTR               equ     1
SYS_INTR                equ     2

%endif

