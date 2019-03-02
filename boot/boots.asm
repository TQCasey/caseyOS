	org	    07c00h

	jmp     short _boot_start		; Start to boot.
	nop

	%include "fat.inc"

	;Macro
	BaseOfLoader		equ	09D00h	;
	OffsetOfLoader		equ	0100h	; LOADER.BIN Loadered  Here [BaseOfLoader:OffsetOfLoader]

	FATTabBase		    equ	50h     ; FAT tab Base
	FATTabOffset		equ	0h	    ; FAT tab Offset
    FATTabLen           equ 0x1200  ; FAT tab length
	;Macro ends

	;VAR IN REAL MODEL
	SrchFromSect	    dw	19		; search from SrchSectNum

	;src
	LoaderStr	        db	"LOADER  BIN"
	;Msg
	BootMsg		        db	"Booting..."
	BootMsgLen	        equ	$ - BootMsg

	NotFoundStr	        db	"No Loader"
	NotFoundStrLen	    equ	$ - NotFoundStr

	FoundStr	        db	"Loader OK"
	FoundStrLen	        equ	$ - FoundStr

	pt		            dw	((80 * 0 + 0) * 2)
	;var ends
;
; code16 start
;

_boot_start:
	mov	    ax,cs
	mov	    ds,ax
	mov	    es,ax

	mov	    ss,ax
	mov	    sp,07c00h

	mov	    si,BootMsg
	mov	    cx,BootMsgLen
	call	dispstr
    call    new_line

	xor	    ah,ah
	xor	    dl,dl
	int	    13h		                ;RST floppy motor


; well ...,follows the most important part of loader,(to load the loader.bin)
; this part if like this (use c statement)
; for ( sect_nr = 0 ; sect_nr < 14 ; sect_nr ++ ) 
; {
;   read_sect (buf,1);
;   for ( char *p = buf ; p < buf + 512 ; p += 0x20 ) 
;   {
;       if ( 0 == __strncmp (p,"LOADER BIN",11))
;           goto __load_file;
;   }
; }
; __load_file:

	mov	    cx,14                   ; 14 sects.... 14 circles
FindOutInRoot:
	;Read Sects
	push	cx		                ; cx(0)

    ; es:bx == buf
    ; start_nr = 19
    ; sect_nr = 1
	mov	    ax,BaseOfLoader
	mov	    es,ax
	mov	    bx,OffsetOfLoader
	mov	    ax,[SrchFromSect]       ; search from 19
	mov	    cl,1		            ; begin to Read (cl) sects from Sects No.[SrchFromSect] 
                                    ; to BaseOfLoader:OffsetOfLoader
	call	ReadSector              ; Finish reading(total 512 bytes)

    ; end read_sect
	mov	    si,LoaderStr	        ; point to "LOADER  BIN"
	mov	    di,OffsetOfLoader       ; ptr to buf
	cld

	mov	    cx,10h		            ; 16 * 32 = 512 Bytes
FindOutInSects:
	push	cx		                ; cx(1)
	                                ; 3rd cycle
	mov	    cx,12
    call    __strncmp
    cmp     cx,0

	pop	    cx		                ; cx(1)
	jz	    load_FAT_table		    ; if equal
	add	    di,20h                  ; p += 0x20
	loop	FindOutInSects	        ; if not equal,find out in the next Dir Entry

	pop	    cx		                ; cx(0)
	inc	    word [SrchFromSect] 	; if not found in this sect ,then next one sect
	loop	FindOutInRoot
                                	;if not found
	mov	    si,NotFoundStr
	mov	    cx,NotFoundStrLen
	call	dispstr
    call    new_line
	jmp	    $		                ; stop here

	                                ; if found
load_FAT_table:
	add     sp,2		            ; cx(0)
	; we need a block to store the FAT tab
	; 'cause there are 9 sects in FAT,so we need  9 * 512 == 4.5KB mem
	; we just read once,no need to read them when called
	push	es

	mov	    ax,FATTabBase
	mov	    es,ax
	mov	    bx,FATTabOffset

	mov	    ax,1		            ; from no.1 sect
	mov	    cl,9		            ; read 9 sects
	call	ReadSector

	pop	    es

	; FAT tab ready

	mov	    si,FoundStr
	mov	    cx,FoundStrLen
	call	dispstr
    call    new_line

	; if found,es:di --> Dest 32Bytes StartAddr
	; We need to get the first sector

	add	    di,01Ah		            ; [es:(di + 01ah)] -- > first cluser
	mov	    ax,word [es:di]	        ; ax -- > first cluser (FAT)

	mov	    bx,OffsetOfLoader

	mov	    cl,1		            ; just one sector
	add	    ax,31
LoadFile:
	call	ReadSector	            ; read a sect to BaseOfLoader:OffsetOfLoader
	; save
	push	ds
	push	si
	;save ax
	push	ax
	mov	    ax,FATTabBase
	mov	    ds,ax
	mov	    si,FATTabOffset
	pop	    ax

	call	GetNextFAT
	;restore
	pop	    si
	pop	    ds

	cmp	    ax,0FFFH
	jz	    endload		            ; if it is the last one,then quit

	add	    ax,31		            ; if not find next FAT

	add	    bx,512		            ; offset += 512
	jmp	    LoadFile	            ; or else continue load file

endload:
	jmp	    BaseOfLoader:OffsetOfLoader 
                                    ; jmp to loader

	%include "rmlib.inc"

times 	510-($-$$)	db	0	        ; fill 510 bytes
                    dw  0xaa55
