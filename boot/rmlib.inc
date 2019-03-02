;----------------------------------------------------------------------------
; funcs:GetNextFAT
; ax	-- > FAT num
; ds:si -- > FAT buf
; before proceed it ,we need make sure the FAT tab already ready
;----------------------------------------------------------------------------

GetNextFAT:
	push	bx
	push	si
	push 	cx
	push 	bp

	sub	    ax,31

	sub 	sp,4
	mov 	bp,sp

	push 	ax		    ; save ax

	shr 	ax,1		; ax/2
	mov 	bx,3		; ax * 3
	mul 	bl		    ;
	add 	si,ax

	mov 	ax,word  [si]
	and 	ax,0FFFH
	mov 	[bp + 0],ax

	mov 	cl,4
	mov 	ax,word [si + 1]
	shr 	ax,cl
	mov 	[bp + 2],ax

	pop 	ax		    ; restore ax
	mov 	bl,2
	div 	bl
	xor 	bx,bx

	mov 	bl,ah
	shl 	bx,1
	mov 	si,bx
	mov 	ax,[bp + si]

	mov 	sp,bp
	add 	sp,4

	pop 	bp
	pop  	cx
	pop	    si
	pop	    bx
	ret

;----------------------------------------------------------------------------
; funcs: dispstr
; ds:si   -- > src
; cx	  -- > len
;----------------------------------------------------------------------------
dispstr:
	push	es
	push	si
	push	di

	mov	    ax,0b800h
	mov	    es,ax
	mov	    di,[pt]

	mov	    ah,0FH
.write:
	lodsb
	stosw
	loop	.write

    mov     word [pt],di

	pop	    di
	pop	    si
	pop	    es
	ret

new_line:
    push    bx
    push    ax

    mov     ax,word [pt]
    mov     bl,160
    div     bl
    and     ax,0xFF
    inc     ax
    mul     bl
    mov     word [pt],ax

    pop     ax
    pop     bx
    ret

;----------------------------------------------------------------------------
; funcs: strcnmp
; cx    --> bytes
; ds:si --> src
; es:di --> dest (this is the spcecial case for offset == 0 )
; cx == 0 means equal ,or not
;----------------------------------------------------------------------------
__strncmp:
	push	si
	push	di
    cld
	repz	cmpsb
	pop	    di
	pop	    si
	ret

;----------------------------------------------------------------------------
; funcs: ReadSector
;from NO.(ax) Sector, read cl Sectors to Buf [es:bx]
;                            ©° V Num = reslt >> 1
;       x           ©° reslt  ©È
; -------------- => ©È        ©¸ Heads = reslt & 1
;    BPB_SecPerTrk  ©¦remain  => StartSects = remain + 1
;----------------------------------------------------------------------------
ReadSector:
	push	bp
	push	ax
	push	bx
	push	cx
	push	dx

	mov	    bp, sp
	sub	    esp, 2

	mov	    byte [bp-2], cl
	push	bx
	mov	    bl, [BPB_SecPerTrk]; 18
	div	    bl
	inc	    ah
	mov	    cl, ah     ; ah:remain
	mov	    dh, al	   ; al:result
	shr	    al, 1	   ; V num
	mov	    ch, al
	and	    dh, 1	   ; Head num
	pop	    bx

	mov	    dl, [BS_DrvNum]
.next:
	mov	    ah, 2
	mov	    al, byte [bp-2]
	int	    13h
	jc	    .next
	add	    esp, 2

	pop	    dx
	pop	    cx
	pop	    bx
	pop	    ax
	pop	    bp
    ret
