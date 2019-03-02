%ifndef __BPB_INC__
%define __BPB_INC__

	;FAT12 HAEDS
	;ColName	    DateType	    ; Exp			    :Must Be (?) Bytes
	BS_OEMName	    DB 'CaseyTTT'	; OEM String,		:8 bytes
	BPB_BytsPerSec	DW 512		    ; Bytes Per Secs	:2 bytes
	BPB_SecPerClus	DB 1		    ; Clusers Per Sec	:1 bytes
	BPB_RsvdSecCnt	DW 1		    ; Reserved Secs		:1 bytes
	BPB_NumFATs	    DB 2		    ; FATS Num		    :1 bytes
	BPB_RootEntCnt	DW 224		    ; Root Dir Secs		:2 bytes
	BPB_TotSec16	DW 2880		    ; Logical Secs		:2 bytes
	BPB_Media	    DB 0xF0		    ; Media Descrip		:1 bytes
	BPB_FATSz16	    DW 9		    ; Sects Per FAT		:2 bytes
	BPB_SecPerTrk	DW 18		    ; Sects Per Tricks	:2 bytes
	BPB_NumHeads	DW 2		    ; Heads			    :2 bytes
	BPB_HiddSec	    DD 0		    ; Hidden secs		:4 bytes
	BPB_TotSec32	DD 0		    ; Total Secs		:4 bytes
	BS_DrvNum	    DB 0		    ; Driver Num		:1 bytes
	BS_Reserved1	DB 0		    ; Not used		    :1 bytes
	BS_BootSig	    DB 29h		    ; boot signal		:1 bytes
	BS_VolID	    DD 0		    ; vol tab num		:4 bytes
	BS_VolLab	    DB 'CaseyOS 1.0'; VolTab		    :11bytes
	BS_FileSysType	DB 'FAT12   '	; FileType Must Be	:8 bytes
	; BPB ends

%endif

