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

int main()
{
	struct rtc_time tm;
	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();

	tm.tm_year = 13;
	tm.tm_mon = 12;
	tm.tm_mday = 23;
	tm.tm_hour = 23;
	tm.tm_min = 20;
	tm.tm_sec = 30;
	tm.tm_wday = 1;

	rtc_settime(&tm);
	
	tm.tm_year = 255;
	tm.tm_mon = 255;
	tm.tm_mday = 255;
	tm.tm_hour = 255;
	tm.tm_min = 255;
	tm.tm_sec = 35;
	rtc_setalarm(&tm);

	while(1)
	{
		rtc_gettime(&tm);
		delayms(1000);
	}
	
	return 0;
}
