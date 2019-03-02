KLIB		=  klib/klib.a
LIB			=  lib.a

# Programs, flags, etc.
ASM		    = nasm
CC	    	= gcc
AR			= ar
ARFLAGS		= rcs
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -I ../include/ -fno-builtin -fno-stack-protector -c


KLIBOBJS	= klib/kstring.o string.o port.o syscalls.o
LIBOBJS		= string.o assert.o printf.o syscalls.o libc.o
# All Phony Targets
.PHONY :all clean lib klib

# Default starting position
all: lib klib

clean :
	rm -f $(KLIBOBJS) $(LIBOBJS) $(KLIB) $(LIB)

# normal lib objects making ...
string.o:	string.c
	$(CC)	$(CFLAGS)	 -o $@ $<
assert.o:	assert.c
	$(CC)	$(CFLAGS)	 -o $@ $<
printf.o:	printf.c
	$(CC)	$(CFLAGS)	 -o $@ $<
port.o:		port.asm
	$(ASM)	$(ASMKFLAGS) -o $@ $<
syscalls.o:	syscalls.c	
	$(CC)	$(CFLAGS)	 -o $@ $<
libc.o:		libc.c
	$(CC)	$(CFLAGS)	 -o $@ $<

# kernel lib objects making ...
klib/kstring.o:	klib/kstring.c
	$(CC)	$(CFLAGS) 	 -o $@ $<

$(LIB):	$(LIBOBJS)	
	$(AR)	$(ARFLAGS)	 $@ $^
$(KLIB):	$(KLIBOBJS)
	$(AR)	$(ARFLAGS)	 $@ $^

lib:	$(LIB)
klib:	$(KLIB)
