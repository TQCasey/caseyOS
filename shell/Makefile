ENTRYPOINT	= 0x1000

BIN			= ../shell.bin
MAP			= shell.map

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -s -I ../include/ -fno-builtin -fno-stack-protector -c
LDFLAGS		= -s -Ttext $(ENTRYPOINT)


# fs objects
# DO REMEMBER entry.o MUST be the first to be linked !!!!!!!!!!
BINOBJS		= entry.o shell.o\
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
shell.o:	shell.c
	$(CC)	$(CFLAGS)		 -o $@ $<

