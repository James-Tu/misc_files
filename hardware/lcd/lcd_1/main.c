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

	lcd_display_pic();
	while(1);

	while(1)
	{
		lcd_clean(0xf800);  //red
		delayms(1000);
		lcd_clean(0x07e0);  //green
		delayms(1000);
		lcd_clean(0x1f);  //blue
		delayms(1000);
	}
	
	return 0;
}
