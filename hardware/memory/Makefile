CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -I include

memory.bin:start.S main.c common.c led.c clock.c lowlevel_init.S
	$(CC) $(CFLAGS) -c -o start.o start.S
	$(CC) $(CFLAGS) -c -o lowlevel_init.o lowlevel_init.S
	$(CC) $(CFLAGS) -c -o main.o main.c
	$(CC) $(CFLAGS) -c -o common.o common.c
	$(CC) $(CFLAGS) -c -o led.o led.c
	$(CC) $(CFLAGS) -c -o clock.o clock.c
	$(LD) -Ttext 0x0 start.o lowlevel_init.o main.o common.o led.o clock.o -o memory.elf
	$(OBJCOPY) -O binary -S memory.elf memory.bin

clean:
	rm -f *.o *.elf *.bin