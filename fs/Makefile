ENTRYPOINT	= 0x0

FS			=  ../fsmain.bin
FMAP        =  fs_map.map

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -s -I ../include/ -fno-builtin -fno-stack-protector -c
LDFLAGS		= -s -Ttext $(ENTRYPOINT)


# fs objects
# DO REMEMBER entry.o MUST be the first to be linked !!!!!!!!!!
FSOBJS		= entry.o fmain.o buffer.o blk_drv.o bitmap.o super.o inode.o blk_rw.o\
			  namei.o file_tbls.o open.o file_rw.o rw.o exec.o\
			  ../lib/port.o ../lib/lib.a

# All Phony Targets
.PHONY :all clean 

# Default starting position
all: $(FS) map

clean :
	rm -f *.o $(FS)

$(FS):		$(FSOBJS)
	$(LD)	$(LDFLAGS)		 -o $@ $^

# fs module param ...
entry.o:	entry.asm
	$(ASM)	$(ASMKFLAGS)	 -o $@ $<
fmain.o:	fmain.c
	$(CC)	$(CFLAGS)	 	 -o $@ $<
buffer.o:	buffer.c
	$(CC)	$(CFLAGS)		 -o $@ $<
blk_drv.o:	blk_drv.c
	$(CC)	$(CFLAGS)		 -o $@ $<
super.o:	super.c
	$(CC)	$(CFLAGS)		 -o $@ $<
inode.o:	inode.c
	$(CC)	$(CFLAGS)		 -o $@ $<
bitmap.o:	bitmap.c
	$(CC)	$(CFLAGS)		 -o $@ $<
namei.o:	namei.c
	$(CC)	$(CFLAGS)		 -o $@ $<
blk_rw.o:	blk_rw.c
	$(CC)	$(CFLAGS)	 	 -o $@ $<
file_rw.o:	file_rw.c
	$(CC)	$(CFLAGS)		 -o $@ $<
open.o:		open.c
	$(CC)	$(CFLAGS)		 -o $@ $<
rw.o:		rw.c
	$(CC)	$(CFLAGS)		 -o $@ $<
file_tbls.o:	file_tbls.c
	$(CC)	$(CFLAGS)		 -o $@ $<
exec.o:		exec.c
	$(CC)	$(CFLAGS)		 -o $@ $<
map:	
	objdump  -t -S   -mi386:intel $(FS)  >$(FMAP)
