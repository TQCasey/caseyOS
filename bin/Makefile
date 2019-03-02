
ENTRYPOINT	= 0x0

BIN			= ../bin.bin
MAP			= bin.map

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -s -I ../include/ -fno-builtin -fno-stack-protector -c
LDFLAGS		= -s -Ttext $(ENTRYPOINT)


# fs objects
# DO REMEMBER entry.o MUST be the first to be linked !!!!!!!!!!
BINOBJS		= entry.o hello.o\
			  ../lib/port.o ../lib/lib.a

# All Phony Targets
.PHONY :all clean map

# Default starting position
all: $(BIN) map

clean :
	rm -f *.o $(BIN)
map:
	objdump -t -S -mi386:intel $(BIN) >> $(MAP)

$(BIN):		$(BINOBJS)
	$(LD)	$(LDFLAGS)		 -o $@ $^

# fs module param ...
entry.o:	entry.asm
	$(ASM)	$(ASMKFLAGS)	 -o $@ $<
hello.o:	hello.c
	$(CC)	$(CFLAGS)		 -o $@ $<
blk_rw.o:	blk_rw.c
# Programs, flags, etc.

