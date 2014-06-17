TOP_DIR = $(shell pwd)

CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -I $(TOP_DIR)/include -Wall -O2 -nostdinc -fno-builtin

export CC LD AR OBJCOPY CFLAGS

OBJS = start.o lowlevel_init.o main.o common.o led.o clock.o serial.o
LIBS = lib/libc.a

DEST = uart.bin
$(DEST):$(OBJS) $(LIBS)
	$(LD) -T hw.lds -o $(@:.bin=.elf) $^
	$(OBJCOPY) -O binary -S $(@:.bin=.elf) $@

clean:
	rm -f $(OBJS) $(DEST) *.elf
	@for subdir in $(dir $(LIBS)); do \
	(make clean -C $$subdir); \
	done

$(LIBS):
	make -C $(dir $@)

%.o:%.c
	${CC} $(CFLAGS) -c -o $@ $<

%.o:%.S
	${CC} $(CFLAGS) -c -o $@ $<