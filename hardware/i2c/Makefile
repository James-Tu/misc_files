TOP_DIR = $(shell pwd)

CROSS_COMPILE = arm-none-linux-gnueabi-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -I $(TOP_DIR)/include -Wall -O2 -nostdinc -fno-builtin
LDFLAGS = -L $(shell dirname `$(CC) -print-file-name=libgcc.a`) -lgcc

export CC LD AR OBJCOPY CFLAGS

OBJS = start.o lowlevel_init.o nand.o main.o common.o led.o clock.o serial.o irq.o button.o
OBJS += timer.o rtc.o lcd.o pic_240x320.o adc_ts.o nor.o watchdog.o i2c.o
LIBS = lib/libc.a

DEST = i2c.bin
$(DEST):$(OBJS) $(LIBS)
	$(LD) -T hw.lds -o $(@:.bin=.elf) $^ $(LDFLAGS)
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