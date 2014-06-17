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

int main()
{	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
	timer4_init();

	lcd_init();
	lcd_clean(0xffff);
	adc_ts_init();
	ts_init();

//	tslib_calibrate();
//	lcd_clean(0xffff);

	nand_init();

	printf("\r\n\n\n watchdog test ... \r\n");
	watchdog_init();

	printf("Please input char 'r'(reboot) or 'c'(crash)\r\n");

	char c;

	while(1)
	{
		c = getc();
		printf("\r\n\r\n");
		if('c' == c)
		{
			printf("system crashed.\r\n");
			memset((void*)0x30000000, 0, 512 * 1024);
		}
		else if('r' == c)
		{
			printf("reboot the system.\r\n");
			watchdog_set_cnt(1);
		}
	}
	

	
	while(1);


	return 0;
}

