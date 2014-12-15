OBJS = kalloc.o \
	   mem.o \
	   vm.o \
	   monitor.o \
	   picirq.o \
	   file.o		\
	   trap.o	\
	   trapasm.o \
	   vectors.o	\
	   buf.o	\
	   ide.o	\
	   main.o
CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump


CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -fvar-tracking -fvar-tracking-assignments -O0 -g -Wall -MD -gdwarf-2 -m32 -fno-omit-frame-pointer
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

xv6.img: bootblock kernel
	dd if=/dev/zero of=xv6.img count=10000
	dd if=bootblock of=xv6.img conv=notrunc
	dd if=kernel of=xv6.img seek=1 conv=notrunc
	
bootblock: bootasm.S bootmain.c sign
	$(CC) -O -nostdinc -I. -c bootmain.c
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c bootasm.S
	$(LD) -N -e start -Ttext 0x7c00 -o bootblock.o bootasm.o bootmain.o
	$(OBJDUMP) -S bootblock.o > bootblock.asm
	$(OBJCOPY) -S -O binary -j .text bootblock.o bootblock
	./sign bootblock
sign: sign.c
	$(CC) sign.c -o sign

kernel: $(OBJS) entry.o kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel entry.o $(OBJS)
	$(OBJDUMP) -S kernel > kernel.asm
clean:
	rm *.asm *.o bootblock sign kernel *.d xv6.img

bochs : xv6.img
	sudo bochs -f .bochsrc
