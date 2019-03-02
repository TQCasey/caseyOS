
ENTRYPOINT	= 0x0

MM			= ../mm.bin
MAP			= mm.map

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -I ../include/ -fno-builtin -fno-stack-protector -c -Wall
LDFLAGS		= -Ttext $(ENTRYPOINT)


# fs objects
# DO REMEMBER entry.o MUST be the first to be linked !!!!!!!!!!
MMOBJS		= entry.o mmain.o\
			  ../lib/port.o ../lib/lib.a

# All Phony Targets
.PHONY :all clean map

# Default starting position
all: $(MM) map

clean :
	rm -f *.o $(MM)
map:
	objdump -t -S -mi386:intel $(MM) >> $(MAP)

$(MM):		$(MMOBJS)
	$(LD)	$(LDFLAGS)		 -o $@ $^

# fs module param ...
entry.o:	entry.asm
	$(ASM)	$(ASMKFLAGS)	 -o $@ $<
mmain.o:	mmain.c
	$(CC)	$(CFLAGS)		 -o $@ $<

