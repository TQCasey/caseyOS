#img,flp
IMG	    	= boot.img
FLOPPY		= /mnt/floppy/
BOOTSECT	= boots.bin
LOADER		= loader.bin
KERNEL		= kernel.bin
FS			= fsmain.bin
TTY			= termio.bin
MM			= mm.bin
BIN			= shell.bin
TST			= bin.bin
KSTRIP		= kernel.bin.s

ALL = $(FS) $(TTY) $(MM) $(KERNEL) $(BIN) $(TST) 

# All Phony Target.PHONY :all clean klib lib fsmain tty dbg 
.PHONY :all clean kernel boot klib lib fsmain term dbg g cpy lk fuck s
# Default starting position
all:  boot lib klib  fsmain term mmain init tst kernel img

s:
	make clean && make cpy && make all && make fuck
clean :
	cd boot		; make clean
	cd kernel	; make clean
	cd fs 		; make clean
	cd lib  	; make clean
	cd term  	; make clean
	cd mm		; make clean
	cd shell	; make clean
	cd bin		; make clean
dbg :
	bochs -f dbg

fuck:
	bochs
g:
	bochs  -qf dbg 2> bochs.txt &
	gdb kernel.bin

cpy:
	rm -f hd.img
	rm -f boot.img
	cp cpy.img ./hd.img
	cp cpy_boot.img ./boot.img

lk:
	rm -f lk.txt
	xxd -a -u -g 1 -c 16 -s 0x9e0200 hd.img >> lk.txt
	vim lk.txt

boot:
	cd boot && make
kernel:
	cd kernel && make
lib:	
	cd lib && make
klib:
	cd lib && make klib
fsmain:
	cd fs && make
term:
	cd term && make
mmain:	
	cd mm && make 
init:
	cd shell && make 
tst:
	cd bin	&& make 

img:
	dd if=$(BOOTSECT) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo 	 mount -o loop 	$(IMG) $(FLOPPY)
	sudo 	 cp $(LOADER) 	$(ALL) $(FLOPPY)
	strip	 $(KERNEL)		-o $(KSTRIP)
	sudo 	 cp $(KSTRIP) 	/mnt/floppy/kernel.bin
	sudo 	 umount 		$(FLOPPY) 
	ctags    -R --fields=+lS
