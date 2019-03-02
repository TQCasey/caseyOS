# Programs, flags, etc.
ASM		    = nasm

# objs 
BOOTSECT	= ../boots.bin
LOADER		= ../loader.bin

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(BOOTSECT) $(LOADER)

clean:
	rm -f $(BOOTSECT) $(LOADER)

$(BOOTSECT):boots.asm
	$(ASM)	$(ASMLFLAGS) -o $@ $<
$(LOADER):	loader.asm
	$(ASM)	$(ASMLFLAGS) -o $@ $<
