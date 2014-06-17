#include<regs-gpio.h>
#include<regs-timer.h>
#include<io.h>
#include<stdio.h>
#include<string.h>
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
	int n, i, vol = 62;
	char c;
	
	n = 10;
	for(i = 0; i < n; i++)
		memcpy((void*)(0x32000000 + i * len), (void*)0x32000000, len);


	printf("start play_audio ... \r\n");
	play_audio((unsigned char*)0x32000000, len * 10);

	while(1)
	{
		c = getc();
		if('+' == c)
		{
			vol += 10;
			vol = (vol > 62) ? 62 : vol;
			vol = (vol < 1) ? 1 : vol;
			uda1341_set_volume(vol);
		}
		else if('-' == c)
		{
			vol -= 10;
			vol = (vol > 62) ? 62 : vol;
			vol = (vol < 1) ? 1 : vol;
			uda1341_set_volume(vol);
		}
		else if('m' == c)
			uda1341_set_mute(1);
		else if('n' == c)
			uda1341_set_mute(0);
	}

	return 0;
}

