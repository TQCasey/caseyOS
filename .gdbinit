file kernel.bin 
target remote localhost:1234
b __kernel_main 
c

layout split 
set disassembly-flavor intel

set print pretty on
