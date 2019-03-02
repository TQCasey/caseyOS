    %include "asm/asmconst.inc"
global _start
global __mem_size,__switched

extern 	GdtPtr,IdtPtr,__tss
extern __kernel_cstart,__kernel_main
extern __curprocs,ret_prc,__sys_call

global  __clk,__keyboard,__hd,__do_syscall
global  __k_reenter

global  save,__procs_restart,re_restart
global  KernelStack

extern  __irq_prc
        [section .data]
        __k_reenter                                  db      0
        __switched                                   db      0
        __mem_size                                   dd      0 

        [section .bss]
        StackSpace	resb	0x8000                        ; 16 KB uninitilazed byte
        KernelStack:

        [section .text]
_start:
        mov     dword [__mem_size],eax 
        mov 	esp,KernelStack
        SGDT	[GdtPtr]                                  ; save gdt to the NewGdtPtr
        call 	__kernel_cstart                           ; copy GDT to New GDT

        LGDT 	[GdtPtr]                                  ; Load New GDT 
        LIDT	[IdtPtr]                                  ; Load New IDT
        jmp 	KERNEL_SELECTOR_CS:CSInit                 ; InitKenrel CS
CSInit:        
        xor 	eax,eax
        mov 	ax,SELECTOR_TSS                           ; Load TSS
        LTR 	ax
        jmp  	__kernel_main                             ; kernel main start

; ========================================================================================
; void save ( void ) ;
; ========================================================================================
save:
        pushad                                            ; save General REGS 
        push	ds
        push 	es
        push 	fs
        push 	gs                                        ; save seg regs
                                                          ; DO NOT touch it !!!!!!
        mov     si,ss                                     ; ss is kernel data seg
        mov     ds,si
        mov 	es,si
        mov     fs,si

        mov     esi,esp                                   ; prepare to return 
        mov     byte [__switched],0
        inc     byte [__k_reenter]                        ; enter == 0,then enter kernel
        jnz     .set_re_start                             ; if already in the kernel stack
        mov     esp,KernelStack                           ; switch to kernel stack
        push    __procs_restart                           ; set restart
        xor     ebp,ebp                                   ; for stack trace
        jmp     [esi + RETADR - P_STACKBASE]              ; ret context 
.set_re_start:
        push    re_restart
        jmp     [esi + RETADR - P_STACKBASE]              ; ret reenter context
; ========================================================================================
; void restart ( void ) ;
; ========================================================================================
__procs_restart:
        mov	    esp, [__curprocs]                         ; switch stack back  
        lldt	[esp + P_LDT_SEL ]                        ; load procs table ldt
        lea 	eax, [esp + P_STACKTOP]                   ; eax = esp + STACKTOP
        mov	    dword [__tss + TSS3_S_SP0], eax           ; set stack 
; ========================================================================================
; void re_restart ( void ) ;
; ========================================================================================
re_restart:
        dec     byte [__k_reenter]                        ; leave && counter --
        pop	    gs
        pop	    fs
        pop	    es
        pop	    ds
        popad
        add	    esp, 4                                    ; skip the ret addr
        iret                                              ; ret to procs
; ========================================================================================
; void save_all ( type ) 
; ========================================================================================
%macro  SAVE    1
        call    save
        mov     byte [esi + 0x5A],%1
%endmacro

; ========================================================================================
; void sys_call ( sys_num (eax) ) 
; ========================================================================================
__do_syscall:   
        SAVE    SYS_INTR
        push    ecx                                       ; pcurprocs
        push    edx                                       ; 
        push    ebx                                       ; buf 
        call 	[__sys_call + eax * 4]                    ; do_task( table_prc [vect_no] );
        add     esp,12                                    ; clean call stack
        push    eax
        call    ret_prc
        add     esp,4
        ret
; ========================================================================================
; void handle_master_irq ( irq_num (eax) ) ; handle master 8259A irqs
; ========================================================================================
%macro  handle_master_irq  1
        SAVE    HARD_INTR
        in      al,M_MASK
        or      al,(1<<%1)
        out     M_MASK,al                                 ; MASK Current IRQ
        mov     al,EOI
        out     M_CTL,al                                  ; send EOI to 8259
        call 	[__irq_prc + %1 * 4]                      ; do_task( table_prc [vect_no] );
        push    eax
        call    ret_prc
        add     esp,4
        in      al,M_MASK
        and     al,~(1<<%1)
        out     M_MASK,al                                 ; Restore Current IRQ
        ret
%endmacro


align 16
__clk: 
        handle_master_irq  0       
align 16
__keyboard: 
        handle_master_irq  1 

; ========================================================================================
; void handle_slaver_irq ( irq_num (eax) ) ; handle master 8259A irqs
; ========================================================================================
%macro  handle_slaver_irq  1
        SAVE    HARD_INTR
        in      al,S_MASK
        or      al,(1<<(%1 - 8))
        out     S_MASK,al                                 ; MASK Current IRQ
        mov     al,EOI
        out     M_CTL,al                                  ; send EOI to 8259 master
        nop     
        out     S_CTL,al                                  ; send EOI to 8259 slaver
        call 	[__irq_prc + %1 * 4]                      ; do_task( table_prc [vect_no] );
        push    eax
        call    ret_prc
        add     esp,4
        in      al,S_MASK
        and     al,~(1<<( %1 - 8))
        out     S_MASK,al                                 ; Restore Current IRQ
        ret
%endmacro


align 16
__hd:     
        handle_slaver_irq  14  
