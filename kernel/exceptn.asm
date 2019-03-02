        %include "asm/asmconst.inc"
global 	__div_err,__debug,__nmi,__breakpt,__overflow,__bounds_chk,__inval_opcode 
global	__copr_unavailable,__double_fault,__copr_seg_overrun,__inval_tss                          
global	__seg_unpresent,__stack_exception,__general_protection,__copr_err                       

extern  __procs_restart,re_restart,KernelStack,__k_reenter,__exceptn_prc

        [section .text]
        ;no err code vect
__div_err:
        push	0                                        ; vector_no	= 0
        jmp     no_err_code
__debug:
        push	1                                        ; vector_no	= 1
        jmp     no_err_code
__nmi:
        push	2                                        ; vector_no	= 2
        jmp     no_err_code
__breakpt:
        push	3                                        ; vector_no	= 3
        jmp     no_err_code
__overflow:
        push	4                                        ; vector_no	= 4
        jmp     no_err_code
__bounds_chk:
        push	5                                        ; vector_no	= 5
        jmp     no_err_code
__inval_opcode:
        push	6                                        ; vector_no	= 6
        jmp     no_err_code
__copr_unavailable:
        push	7                                        ; vector_no	= 7
        jmp     no_err_code
__double_fault:
        push	8                                        ; vector_no	= 8
        jmp     no_err_code
__copr_seg_overrun:
        push	9                                        ; vector_no	= 9
        jmp     no_err_code


no_err_code:
        xchg    eax,[esp]                                 ; eax (err_code),(esp) == eax
        pushad                                            ; save General REGS to caller stack space
        push	ds
        push 	es
        push 	fs
        push 	gs                                        ; save seg regs
                                                          ; DO NOT touch it !!!!!!
        mov     dx,ss                                     ; ss is kernel data seg
        mov     ds,dx
        mov 	es,dx
        mov     fs,dx

        mov     esi,esp                                   ; get eax from ret_addr
        mov     ebx,[esi + 0x30]                          ; get eax
        mov     ebp,esi
        mov     [esi + EAXREG - P_STACKBASE],ebx          ; recover eax
        inc     byte [__k_reenter]
        jnz     .set_re_start
        mov     esp,KernelStack
        push    __procs_restart
        jmp     .next_s
.set_re_start:
        push    re_restart
.next_s:
        shl     eax,2                                       ; eax * 4
        call    [__exceptn_prc + eax]
        ret

;==================================================================================
; error_code
;==================================================================================
%macro  handle_err_code  1
        xchg    eax,[esp]                                 ; eax (err_code),(esp) == eax
        pushad                                            ; save General REGS to caller stack space
        push	ds
        push 	es
        push 	fs
        push 	gs                                        ; save seg regs
                                                          ; DO NOT touch it !!!!!!
        mov     dx,ss                                     ; ss is kernel data seg
        mov     ds,dx
        mov 	es,dx
        mov     fs,dx

        mov     esi,esp                                   ; get eax from ret_addr
        mov     ebx,[esi + 0x30]                          ; get eax
        mov     ebp,esi
        mov     [esi + EAXREG - P_STACKBASE],ebx          ; recover eax
        inc     byte [__k_reenter]
        jnz     .set_re_start
        mov     esp,KernelStack
        push    __procs_restart
        jmp     .next_s
.set_re_start:
        push    re_restart
.next_s:
        push    eax                                      ; err_code
        call    [__exceptn_prc + %1*4]
        add     esp,4
        ret
%endmacro

__inval_tss:            
    handle_err_code 0x0A
__seg_unpresent:        
    handle_err_code 0x0B
__stack_exception:      
    handle_err_code 0x0C
__general_protection:   
    handle_err_code 0x0D
__copr_err:             
    handle_err_code 0x10
