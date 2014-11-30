CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump

CFLAGS = -fno-builtin -O2 -Wall -MD

xv6.img: bootblock
	dd if=/dev/zero of=xv6.img count=10000
	dd if=bootblock of=xv6.img conv=notrunc
	
bootblock: bootasm.S bootmain.c sign
	$(CC) -O -nostdinc -I. -c bootmain.c
	$(CC) -nostdinc -I. -c bootasm.S
	$(LD) -N -e start -Ttext 0x7c00 -o bootblock.o bootasm.o bootmain.o
	$(OBJDUMP) -S bootblock.o > bootblock.asm
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	./sign bootblock
sign: sign.c
	$(CC) sign.c -o sign
clean:
	rm *.asm *.o bootblock sign *.img
