CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -I include

led.bin:start.S main.c common.c led.c
	$(CC) $(CFLAGS) -c -o start.o start.S
	$(CC) $(CFLAGS) -c -o main.o main.c
	$(CC) $(CFLAGS) -c -o common.o common.c
	$(CC) $(CFLAGS) -c -o led.o led.c
	$(LD) -Ttext 0x0 start.o main.o common.o led.o -o led.elf
	$(OBJCOPY) -O binary -S led.elf led.bin

clean:
	rm -f *.o *.elf *.bin