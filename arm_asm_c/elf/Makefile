CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

sample.bin:start.S main.c
	$(CC) -c -o start.o start.S
	$(CC) -c -o main.o main.c
	$(LD) -Ttext 0x0 start.o main.o -o sample.elf
	$(OBJCOPY) -O binary -S sample.elf sample.bin

clean:
	rm -f *.o *.elf *.bin