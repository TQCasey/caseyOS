    %include "asm/asmconst.inc"
global __page_fault
extern __do_no_page,__procs_restart,re_restart,KernelStack,__k_reenter

; prototypes : void __do_page_fault ( __u32 err_code ) ;!!!!!!!!!!!!!!!
; we can not schedule this procedure.so we don't use __procs_restart();
; because we need to do nothing but the next instruction ,hope it works 

        [section .text]
__page_fault:
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
        mov     [esi + EAXREG - P_STACKBASE],ebx          ; recover eax
        inc     byte [__k_reenter]
        jnz     .set_re_start
        mov     esp,KernelStack
        push    __procs_restart
        jmp     .next
.set_re_start:
        push    re_restart
.next:
        test    eax,1
        jne     wt_denied

        mov     edx,cr2
        push    edx                                       ; addr
        call    __do_no_page
        add     esp,4

        jmp     pf_ret
wt_denied:
        inc     byte [gs:4]
        hlt
pf_ret:
        ret
