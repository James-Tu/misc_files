#include<regs-gpio.h>
#include<regs-timer.h>
#include<io.h>
#include<stdio.h>
#include"irq.h"
#include"timer.h"
#include"serial.h"
#include"button.h"
#include"rtc.h"
#include"common.h"
#include"lcd.h"
#include"adc_ts.h"
#include"nor.h"
#include"nand.h"

int main()
{	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();

	lcd_init();
	lcd_clean(0xffff);
	adc_ts_init();
	ts_init();

//	tslib_calibrate();
//	lcd_clean(0xffff);

	nand_init();
#if 1	
	nand_read_id();
	
	while(1);
#endif

	printf("\r\n\n");
	printf("===============================================\r\n");
	printf("              NAND FLASH PROGRAMMING            \r\n");
	
	unsigned int size = 512 * 1024; // 512KB
	nand_read_id();

	printf("erase entire flash, waiting ... \r\n");

	int i;
	for(i = 0; i < 2048; i++)
		nand_erase_block(i);
	
	printf("start program ... \r\n");
	nand_write_bytes(0x0, (unsigned char *)0x31000000, size);
	printf("program end ... \r\n");
	printf("===============================================\r\n");
	while(1);
	
	return 0;
}

