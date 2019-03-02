# Programs, flags, etc.
ENTRYPOINT  = 0x115000
ASM		    = nasm
CC	    	= gcc
LD		    = ld
ASMKFLAGS	= -I ../include/ -f elf
CFLAGS		= -g -I ../include/ -fno-builtin -fno-stack-protector -c
LDFLAGS		= -g -Ttext $(ENTRYPOINT)

# objs 
KERNEL		= ../kernel.bin
KMAP		= kernel.map

# kernel objects 
KOBJS		= kernel.o dtcntl.o kmain.o excpcalls.o irq.o  sched.o exceptn.o syscall.o \
			  creatp.o exit.o wait.o panic.o keybd.o hdsk.o kconio.o page.o page_map.o \
			  main.o setup.o
IPC			= send.o post.o pick.o reply.o msgcall.o

KLIB		= ../lib/klib/klib.a

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(KERNEL) map

clean:
	rm -f *.o $(KERNEL)

$(KERNEL):	$(KOBJS) $(IPC) $(KLIB)
	$(LD)	$(LDFLAGS)		-o $@ $^

# kernel objects making....
kernel.o:		kernel.asm
	$(ASM)	$(ASMKFLAGS) -o $@ $<
kmain.o: 		kmain.c
	$(CC)	$(CFLAGS) 	 -o $@ $<
dtcntl.o:		dtcntl.c
	$(CC)	$(CFLAGS)	 -o $@ $<
excpcalls.o:	excpcalls.c
	$(CC)	$(CFLAGS)	 -o $@ $<
irq.o:			irq.c
	$(CC)	$(CFLAGS)	 -o $@ $<
sched.o: 		sched.c
	$(CC)	$(CFLAGS) 	 -o $@ $<
syscall.o:		syscall.c
	$(CC)	$(CFLAGS)	 -o $@ $<
sys.o:			sys.asm
	$(ASM)	$(ASMKFLAGS) -o $@ $<
creatp.o:		creatp.c
	$(CC)	$(CFLAGS)	 -o $@ $<
keybd.o:		keybd.c
	$(CC)	$(CFLAGS)	 -o $@ $<
exceptn.o:		exceptn.asm
	$(ASM)	$(ASMKFLAGS) -o $@ $<
hdsk.o:			hdsk.c
	$(CC)	$(CFLAGS)	 -o $@ $<
kconio.o:		kconio.c
	$(CC)	$(CFLAGS)	 -o $@ $<
exit.o:			exit.c
	$(CC)	$(CFLAGS)	 -o $@ $<
wait.o:			wait.c
	$(CC)	$(CFLAGS)	 -o $@ $<
panic.o:		panic.c		
	$(CC)	$(CFLAGS)	 -o $@ $<

# page #
page.o:		page/page.asm
	$(ASM)	$(ASMKFLAGS) -o $@ $<
page_map.o:	page/page_map.c
	$(CC)	$(CFLAGS)	 -o $@ $<

# task0 #
main.o:		task0/main.c
	$(CC)	$(CFLAGS)	 -o $@ $<
setup.o:	task0/setup.c
	$(CC)	$(CFLAGS)	 -o $@ $<




msgcall.o:	../ipc/msgcall.c
	$(CC)	$(CFLAGS)	 -o $@ $<
send.o:		../ipc/send.c
	$(CC)	$(CFLAGS)	 -o $@ $<
post.o:		../ipc/post.c
	$(CC)	$(CFLAGS)	 -o $@ $<
pick.o:		../ipc/pick.c
	$(CC)	$(CFLAGS)	 -o $@ $<
reply.o:	../ipc/reply.c
	$(CC)	$(CFLAGS)	 -o $@ $<





map:
	objdump  -t -S   -mi386:intel $(KERNEL)  >$(KMAP)
