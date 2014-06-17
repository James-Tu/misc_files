CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -I include

clock.bin:start.S main.c common.c led.c clock.c
	$(CC) $(CFLAGS) -c -o start.o start.S
	$(CC) $(CFLAGS) -c -o main.o main.c
	$(CC) $(CFLAGS) -c -o common.o common.c
	$(CC) $(CFLAGS) -c -o led.o led.c
	$(CC) $(CFLAGS) -c -o clock.o clock.c
	$(LD) -Ttext 0x0 start.o main.o common.o led.o clock.o -o clock.elf
	$(OBJCOPY) -O binary -S clock.elf clock.bin

clean:
	rm -f *.o *.elf *.bin