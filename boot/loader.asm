    org	0100h
	jmp     _loader_start			; Start to load kernel module.

	%include "fat.inc"
	%include "pmhdr.inc"

	BaseOfLoader	equ	09D00h	    ;
	OffsetOfLoader	equ	0100h	    ; 0x9d000 - 0x9fc00 for loader.bin

	KernelEntry		equ	115000h     ; 1M + 84K for kernel space 
    kernel_size     equ 0D000h      ; 128 KB for kernel

	BaseOfKernel	equ	07D00h	    ;
	OffsetOfKernel	equ	0	        ; 0x7d000 - 0x9d000 for kernel.bin

    base_of_fs      equ 05d00h      ; 0x5d000 - 0x7d000 for fs.bin
    off_fs          equ 0

    base_of_tty     equ 03d00h      ; 0x3d000 - 0x5d000 for tty.bin
    off_tty         equ 0
    
    base_of_mm      equ 01d00h      ; 0x1d000 - 0x3d000 for mm.bin
    off_mm          equ 0

    base_of_shell   equ 0d00h       ; 0x0d000 - 0x1d000 64 KB for shell.bin
    off_shell       equ 0

    base_of_tst     equ 0100h       ; 0x1000 - 0xd000 for tst.bin
    off_tst         equ 0      

	FATTabBase		equ	50h         ; FAT tab Base
	FATTabOffset	equ	0h	        ; FAT tab Offset
    FATTabLen       equ 0x1200      ; FAT tab length (total 4.5KB)

	page_dir_table	equ	100000h	    ; page_dir_table from 1M
    page_table      equ 101000h     ; 1M + 4K

	GDT:		    Descriptor	0	    ,	0	    ,   0
	FLAT_C:		    Descriptor	0	    ,   0FFFFFH	,	DA_CR |DA_32|DA_LIMIT_4K    ; 0~4G code
	FLAT_RW		    Descriptor	0	    ,	0FFFFFH	,	DA_DRW|DA_32|DA_LIMIT_4K	; 0~4G data
	VIDEO:		    Descriptor	0B8000H ,	0FFFFH	,	DA_DRW|DA_32|DA_DPL3


	GdtLen		    equ	$ - GDT
	GdtPtr		    dw	GdtLen - 1  ; GdtLimit
			        dd	BaseOfLoader * 16 + GDT

	;selector
	SelectorFlat_C	equ	FLAT_C - GDT; 8h
	SelectorFlat_RW	equ	FLAT_RW- GDT; 10h
	SelectorVideo	equ	VIDEO  - GDT; 18h
	

	;VAR IN REAL MODEL
	RootTotalSects	equ	14		    ; Root Sects
	SrchFromSect	dw	19		    ; search from SrchSectNum
	;src

    ;   string source name
	src_name	    db	"KERNEL  BIN"
	          	    db	"FSMAIN  BIN"
                    db  "TERMIO  BIN"
                    db  "MM      BIN"
                    db  "SHELL   BIN"
                    db  "BIN     BIN"

    ;   message string 
	not_found	    db	"PANIC !==> NO  "
    not_found_len   equ $ - not_found 
    loading         db  "Loading "
    loading_len     equ $ - loading
    load_ok         db  " Loaded OK !"
    load_ok_len     equ $ - load_ok

	pt		        dw	((80 * 2 + 0) * 2)
	;var ends
;
; code16 start
;

_loader_start:
	mov	    ax,cs
	mov	    ds,ax
	mov	    es,ax

	mov	    ss,ax
	mov	    sp,0100h

	mov	    ebx, 0
	mov	    di, _MemChkBuf
.loop:
	mov	    eax, 0E820h
	mov	    ecx, 20
	mov	    edx, 0534D4150h
	int	    15h
	jc	    MEM_CHK_FAIL
	add	    di, 20
	inc	    dword [_dwMCRNumber]
	cmp	    ebx, 0
	jne	    .loop
	jmp	    MEM_CHK_OK
MEM_CHK_FAIL:
	mov	    dword [_dwMCRNumber], 0
MEM_CHK_OK:
	xor	    ah,ah
	xor	    dl,dl
	int	    13h		                ; RST floppy motor

    push    0
    push    OffsetOfKernel
    push    BaseOfKernel
    call    load
    add     sp,6

    push    1
    push    off_fs
    push    base_of_fs
    call    load
    add     sp,6

    push    2
    push    off_tty
    push    base_of_tty
    call    load
    add     sp,6

    push    3
    push    off_mm
    push    base_of_mm
    call    load
    add     sp,6

    push    4
    push    off_shell
    push    base_of_shell
    call    load
    add     sp,6

    push    5
    push    off_tst
    push    base_of_tst
    call    load
    add     sp,6

	call	KillMotor
	LGDT	[GdtPtr]
	cli

	;open a20
	in	    al,92H
	or	    al,00000010B
	out	    92H,al

	;pe = 1
	mov	    eax,CR0
	or	    eax,1
	mov	    CR0,eax

    mov     cx,word [pt]

    ; jmp to protect mode 
	jmp	    dword SelectorFlat_C:(BaseOfLoader * 16 + CODE32_START)
;----------------------------------------------------------------------------
; func: KillMotor ();
;----------------------------------------------------------------------------
KillMotor:
	push	dx
	mov	    dx, 03F2h
	mov	    al, 0
	out	    dx, al
	pop	    dx
	ret
; ============================================================================================
; void load ( unsigned short base ,unsigned short offset ,unsigned short src_index ) ;
; ============================================================================================
load:
    push    bp
    mov     bp,sp

    sub     sp,2                    ; local var for src str

    push    ax
    push    bx
    push    cx
    push    dx
    push    si
    push    di
    push    es

    mov     es,word [bp + 4]        ; get base
    mov     di,word [bp + 6]        ; get off
    mov     ax,word [bp + 8]        ; get index 
    

    mov     si,src_name             
    mov     dl,11
    mul     dl
    add     si,ax                   ; get src string 

    mov     word [bp - 2],si        ; store 

	mov	    cx,RootTotalSects       ; 14 sects..., 14 circles
.FindOutInRoot:
	push	cx		                ; cx(0)
	mov	    bx,word [bp + 6]        ; get offset 
	mov	    ax,[SrchFromSect]       ; search from 19
	mov	    cl,1		            ; begin to Read (cl) sects from Sects No.[SrchFromSect] 
                                    ; to BaseOfLoader:OffsetOfLoader
	call	ReadSector
	                                ; Finish reading(total 512 bytes)
	cld
	mov	    cx,10h		            ; 16 * 32 = 512 Bytes
.FindOutInSects:
	push	cx		                ; cx(1)
	                                ; 3rd cycle
	mov	    cx,12
	call	__strncmp
    cmp     cx,0
	pop	    cx		                ; cx(1)
	jz	    .found		            ; if equal
	add	    di,20h
	loop	.FindOutInSects	        ; if not equal,find out in the next Dir Entry
	pop	    cx		                ; cx(0)
	inc	    word [SrchFromSect]
	loop	.FindOutInRoot
			                    	; if not found in this sect ,then next one sect
	                                ; if not found
	mov	    si,not_found
	mov	    cx,not_found_len
	call	dispstr

    mov     si,word [bp - 2]        ; show string 
    mov     cx,11
    call    dispstr

    call    new_line

	jmp	    $		                ; stop here

.found:
	pop	    cx		                ; cx(0)

	mov	    si,loading
	mov	    cx,loading_len
	call	dispstr
    mov     si,word [bp - 2]
    mov     cx,11
    call    dispstr
    call    new_line

	; if found,es:di --> Dest 32Bytes StartAddr
	; We need to get the first sector
	add	    di,01Ah		            ; [es:(di + 01ah)] -- > first cluser
	mov	    ax,word [es:di]	        ; ax -- > first cluser (FAT)
	mov	    bx,word [bp + 6]

	mov	    cl,1		            ; just one sector
	add	    ax,31
.LoadFile:
	call	ReadSector	            ; read a sect to BaseOfLoader:OffsetOfLoader
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
	jz	    .endload		        ; if it is the last one,then quit
	add	    ax,31		            ; if not find next FAT
	add	    bx,512		            ; offset += 512
	jmp	    .LoadFile	            ; or else continue load file

.endload:                          ; from now on we finished loading kernel module
    mov     si,word [bp - 2]
    mov     cx,11
    call    dispstr

	mov	    si,load_ok
	mov	    cx,load_ok_len
	call	dispstr

    call    new_line

    pop     es
    pop     di
    pop     si
    pop     dx
    pop     cx
    pop     bx
    pop     ax

    mov     sp,bp
    pop     bp
    ret


	%include "rmlib.inc"


; now we entered the protected mode 

[section .code32]
align 32
[bits 32]

CODE32_START:
	mov	    ax,SelectorVideo
	mov	    gs,ax

	mov	    ax,SelectorFlat_RW
	mov	    ds,ax
	mov	    es,ax
	mov	    fs,ax
	mov	    ss,ax
	mov	    esp,TopOfStack

    xor     ebx,ebx
    mov     bx,cx
    mov     dword [dwDispPos],ebx

	push	PMmsg
	call	DispStr
	add	    esp,4

	PUSH	RamSize
	call	DispStr
	add	    esp,4

	call	GetMemSize
    push    0x100000
    push    0
    push    0x100000
    call    memset
    add     esp,12
	call	SetupPaging

	push	PageMsg
	call	DispStr
	add	    esp,4


	push	InitKernelStr
	call	DispStr
	add	    esp,4

	call	InitKernel

	push	InitReady
	call	DispStr
	add	    esp,4

    ; we gonna need some usefull info from real mode
    ; place them into regs is a good idear

    mov     eax,dword [dwMemSize]
	jmp	    SelectorFlat_C:KernelEntry	    ; we goto kernel

GetMemSize:
	mov	    esi, MemChkBuf
	mov	    ecx, [dwMCRNumber]              ; for(int i=0;i<[MCRNumber];i++)
.loop:				                        ; {
	push	ecx
	mov	    ecx,5		                    ;	for(int j=0;j<5;j++)
	mov	    edi, ARDStruct	                ;	{
.1:				                            ;
	lodsd			                        ;
	stosd			                        ;		ARDStruct[j*4] = MemChkBuf[j*4];
	loop	.1		                        ;	}
	pop	    ecx                             ; 
	mov	    eax, [dwBaseAddrLow]            ;
	add	    eax, [dwLengthLow]              ;
	cmp	    eax, [dwMemSize]                ;	if(BaseAddrLow + LengthLow > MemSize)
	jb	    .2		                        ;	{
	mov	    [dwMemSize], eax                ;			MemSize = BaseAddrLow + LengthLow
.2:				                            ;	}
	loop	.loop		                    ; }
				                            ; /* the max mem_size for us is 64MB */
    cmp     dword [dwMemSize],0x4000000     ; if ( mem_size > 64 ) 
    jb      .less_512MB                     ;   mem_size = 64
    mov     dword [dwMemSize],0x4000000     ; 
.less_512MB:                                ; 
	push	dword [dwMemSize]               ; 
	call	DispInt		                    ; DispInt(MemSize);
	add	    esp, 4		                    ;
	ret

SetupPaging:
	mov	    eax,dword [dwMemSize]
    shr     eax,12                          ; let it be 4KB alignment

    xor     edx,edx
    mov     ebx,1024                        ; let it be 4MB alignment
	div	    ebx                             ; edx (ramin) eax (result)
    cmp     edx,0                           ; if 4MB alignment 
    jz      .4MB_alignment
    add     eax,1                           ; if not ,add a page_dir_objs
.4MB_alignment:
	mov	    ecx,eax

	mov	    ax,SelectorFlat_RW
	mov	    es,ax
    mov     ds,ax
    cld

	mov 	edi,page_dir_table

	xor	    eax,eax
    mov     eax,page_table | PG_P | PG_USU | PG_RWW 
.1:
	stosd
	add	    eax, 4096
	loop	.1

	; page_table init...
    mov     ecx,dword [dwMemSize]
    shr     ecx,12                          ; 4KB alignment,(ecx) == how many pages we have 
    mov     edi,page_table
	xor	    eax, eax
	mov	    eax, PG_P | PG_USU | PG_RWW
.2:
	stosd
	add	    eax, 4096
	loop	.2
    ; start up paging....
	mov	    eax, page_dir_table
	mov	    cr3, eax
	mov	    eax, cr0
	or	    eax, 80000000h
	mov	    cr0, eax
	ret

InitKernel:
	xor	    esi,esi
	xor	    ecx,ecx
	mov	    cx,  word [BaseOfKernel * 16 + 02Ch]	; e_phnum
	mov	    esi,dword [BaseOfKernel * 16 + 1CH ]	; e_phoff'
	add	    esi,BaseOfKernel * 16			        ; e_phoff
_cpy:
	mov	    eax,[esi + 0]				            ; if p_type == PT_NULL
	cmp	    eax,0
	jz	    _nextph

	mov	    eax,dword [esi + 04H]			        ; p_offset'
	add	    eax,BaseOfKernel * 16			        ; p_offset


	push	dword [esi + 010h]			            ; size
	push	eax					                    ; src offset
	push	dword [esi + 08h ]			            ; dest offset
	call	memcpy
	add	    esp,12
_nextph:
	add	    esi,32					                ; next ph
	loop	_cpy

	ret

	%include "pmlib.inc"

	;data sect
	[section .data32]
	align 32
	[bits 32]
	DATA32_START:

	;msg
	_PMmsg:				    db	"Protect Mode Actived!",0ah,0
	_PageMsg:			    db	0ah,"Paging Ready!",0AH,0
	_RamSize			    db	"Total RAM Size:",0
	_InitKernelStr			db	"Inint Kernel...",0ah,0
	_InitReady			    db	"Kernel Ready!",0ah,0
	;var
	_dwMCRNumber:			dd	0	                ; Memory Check Result
	_dwDispPos:			    dd	(80 * 8 + 0) * 2	;
	_dwMemSize:			    dd	0
	_ARDStruct:			                            ; Address Range Descriptor Structure
		_dwBaseAddrLow:		dd	0
		_dwBaseAddrHigh:	dd	0
		_dwLengthLow:		dd	0
		_dwLengthHigh:		dd	0
		_dwType:		    dd	0

	_MemChkBuf:	times 1024	db	0

	PMmsg			        equ	_PMmsg		    + BaseOfLoader * 16
	PageMsg			        equ	_PageMsg	    + BaseOfLoader * 16
	RamSize			        equ	_RamSize	    + BaseOfLoader * 16
	InitKernelStr		    equ	_InitKernelStr	+ BaseOfLoader * 16
	InitReady		        equ	_InitReady	    + BaseOfLoader * 16
	;
	dwMCRNumber		        equ	_dwMCRNumber	+ BaseOfLoader * 16
	dwDispPos		        equ	_dwDispPos	    + BaseOfLoader * 16
	dwMemSize		        equ	_dwMemSize	    + BaseOfLoader * 16
	ARDStruct		        equ	_ARDStruct	    + BaseOfLoader * 16
		dwBaseAddrLow	    equ	_dwBaseAddrLow	+ BaseOfLoader * 16
		dwBaseAddrHigh	    equ	_dwBaseAddrHigh	+ BaseOfLoader * 16
		dwLengthLow	        equ	_dwLengthLow	+ BaseOfLoader * 16
		dwLengthHigh	    equ	_dwLengthHigh	+ BaseOfLoader * 16
		dwType		        equ	_dwType		    + BaseOfLoader * 16
	MemChkBuf		        equ	_MemChkBuf	    + BaseOfLoader * 16

	StackSpace:	times 1024	db	0
	TopOfStack	equ	BaseOfLoader * 16 + $	        ;


	Data32Len	equ $	- DATA32_START
