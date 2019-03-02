ENTRYPOINT	= 0x00

TERM		=  ../termio.bin
TERMMAP     =  term_map.map

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -I ../include/ -fno-builtin -fno-stack-protector -c
LDFLAGS		= -Ttext $(ENTRYPOINT)


# fs objects
# DO REMEMBER entry.o MUST be the first to be linked !!!!!!!!!!
TERMOBJS	= entry.o term.o decode.o rw_con.o codemap.o rw_tty.o\
			  ../lib/syscalls.o \
			  ../lib/lib.a 
#../lib/port.o ../lib/lib.a

# All Phony Targets
.PHONY :all clean 

# Default starting position
all: $(TERM) map

clean :
	rm -f *.o $(TERM)

$(TERM):	$(TERMOBJS)
	$(LD)	$(LDFLAGS)		 -o $@ $^

entry.o:				entry.asm
	$(ASM)	$(ASMKFLAGS)	 -o $@ $<
term.o:					term.c
	$(CC)	$(CFLAGS)	 	 -o $@ $<
codemap.o:				codemap.c
	$(CC)	$(CFLAGS)		 -o $@ $<
decode.o:				decode.c
	$(CC)	$(CFLAGS)		 -o $@ $<
rw_con.o:				rw_con.c
	$(CC)	$(CFLAGS)		 -o $@ $<
rw_tty.o:				rw_tty.c
	$(CC)	$(CFLAGS)		 -o $@ $<

map:	
	objdump  -t -S  -mi386:intel  $(TERM)  >$(TERMMAP)
