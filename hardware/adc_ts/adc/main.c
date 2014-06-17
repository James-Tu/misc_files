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

int main()
{	
	unsigned int adc;
	float vol;
	int n;
	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();

	lcd_init();
	lcd_clean(0xffff);
	adc_ts_init();

	while(1)
	{
		adc = read_adc(0);
		vol = adc * 3.3 / 0x3ff;
		n = (vol - (int)vol) * 1000;
		
		printf("AIN0: adc = %u	voltage:%d.%03d\r\n", adc, (int)vol, n);
		delayms(1000);
	}
	
	return 0;
}
