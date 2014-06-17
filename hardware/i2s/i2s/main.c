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
#include"watchdog.h"
#include"i2c.h"
#include"cmif.h"
#include"i2s.h"

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

	i2c_init();

	i2s_init();
	uda1341_init();

	int len = 456 * 1024;

	len = 1024 * 1024;
	
//	while(1)
	printf("start record_audio ... \r\n");
	record_audio((unsigned char*)0x32000000, len);
	
	printf("start play_audio ... \r\n");
	while(1)
		play_audio((unsigned char*)0x32000000, len);

	while(1);


	return 0;
}

