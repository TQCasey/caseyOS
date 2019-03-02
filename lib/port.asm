global  wrports
global  rdports


    [SECTION .text]
; -----------------------------------------------------------------------
; size_t wrports ( PORT port,void *buf,size_t len );
; -----------------------------------------------------------------------
wrports:
	push 	ebp
	mov 	ebp,esp
	push 	esi
    xor     eax,eax              ; eax = 0 
	mov 	edx,[ebp + 8 ]       ; port
	mov 	esi,[ebp + 12]       ; buf 
    mov     ecx,[ebp + 16]       ; len
    cld
    test    cl,1
    jz      .even
    rep     outsb
    jmp     .set_ret
.even:
    shr     ecx,1
    rep     outsw
.set_ret:
    mov     eax,esi
    sub     eax,[ebp + 12]       ; return (cnt);
    pop     esi
	pop 	ebp
	ret

; -----------------------------------------------------------------------
; void* rdports ( PORT port,void *buf,size_t len );
; -----------------------------------------------------------------------
rdports:
	push 	ebp
	mov 	ebp,esp
    push    edi
    xor     eax,eax             ; eax = 0 
	mov 	edx,[ebp + 8 ]	    ; port 
    mov     edi,[ebp + 12]      ; buf
    mov     ecx,[ebp + 16]      ; len
    cld
    test    cl,1
    jz      .even
    rep     insb
    jmp     .set_ret
.even:
    shr     ecx,1
    rep     insw
.set_ret:
    mov     eax,[ebp + 12]      ; return (buf);
    pop     edi
	pop 	ebp
	ret
