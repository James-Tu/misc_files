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

	char c;
	unsigned int WinHorOfst = 0, WinVerOfst = 0;

	cmif_init();
	ov9650_init();
	cmif_config(LCD_WIDTH, LCD_HEIGHT, WinHorOfst, WinVerOfst);
	start_capture();

	while(1)
	{
		c = getc();
		if('+' == c)
		{
			if(WinHorOfst == 160)
				continue;
				
			WinHorOfst += 16;
			WinVerOfst += 16;
			stop_capture();
			cmif_config(LCD_WIDTH, LCD_HEIGHT, WinHorOfst, WinVerOfst);
			start_capture();
		}
		else if('-' == c)
		{
			if(WinHorOfst == 0)
				continue;
				
			WinHorOfst -= 16;
			WinVerOfst -= 16;
			stop_capture();
			cmif_config(LCD_WIDTH, LCD_HEIGHT, WinHorOfst, WinVerOfst);
			start_capture();
		}
		else if('p' == c)
			stop_capture();
		else if('s' == c)
			start_capture();
	}

	while(1);


	return 0;
}

