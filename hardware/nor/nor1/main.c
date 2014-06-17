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

	nor_read_id();

	nor_cfi_query();
	while(1);


	printf("erase entire flash \r\n");
	nor_sector_erase(0xff);

	printf("start program ... \r\n");
	nor_program_bytes(0x0, (unsigned short *)0x30000000, 256 * 1024);
	printf("program end ... \r\n");
	
	while(1);
	
	return 0;
}

