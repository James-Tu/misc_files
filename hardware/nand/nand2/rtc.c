#include<stdio.h>
#include<io.h>
#include<regs-clock.h>
#include<regs-rtc.h>
#include"clock.h"
#include"rtc.h"

void rtc_init()
{
	clk_enable(CLKCON_RTC, 1);
	writeb(0, RTCCON);
}

void rtc_settime(struct rtc_time *tm)
{
	writeb(1, RTCCON);

	printf("set time: %d/%d/%d %d:%d:%d  weekday:%d\r\n",
			tm->tm_year, tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			tm->tm_wday);

	writeb(bin2bcd(tm->tm_year), BCDYEAR);
	writeb(bin2bcd(tm->tm_mon), BCDMON);
	writeb(bin2bcd(tm->tm_mday), BCDDATE);
	writeb(bin2bcd(tm->tm_hour), BCDHOUR);
	writeb(bin2bcd(tm->tm_min), BCDMIN);
	writeb(bin2bcd(tm->tm_sec), BCDSEC);
	writeb(bin2bcd(tm->tm_wday), BCDDAY);

	writeb(0, RTCCON);
}

void rtc_gettime(struct rtc_time *tm)
{
	writeb(1, RTCCON);

	tm->tm_year = bcd2bin(readb(BCDYEAR));
	tm->tm_mon = bcd2bin(readb(BCDMON));
	tm->tm_mday = bcd2bin(readb(BCDDATE));
	tm->tm_hour = bcd2bin(readb(BCDHOUR));
	tm->tm_min = bcd2bin(readb(BCDMIN));
	tm->tm_sec = bcd2bin(readb(BCDSEC));
	tm->tm_wday = bcd2bin(readb(BCDDAY));

	if(0 == tm->tm_sec)
	{
		tm->tm_year = bcd2bin(readb(BCDYEAR));
		tm->tm_mon = bcd2bin(readb(BCDMON));
		tm->tm_mday = bcd2bin(readb(BCDDATE));
		tm->tm_hour = bcd2bin(readb(BCDHOUR));
		tm->tm_min = bcd2bin(readb(BCDMIN));
		tm->tm_sec = bcd2bin(readb(BCDSEC));
		tm->tm_wday = bcd2bin(readb(BCDDAY));	
	}
	
	writeb(0, RTCCON);

	printf("get time: %d/%d/%d %d:%d:%d  weekday:%d\r\n",
			tm->tm_year, tm->tm_mon, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec,
			tm->tm_wday);
}

void rtc_setalarm(struct rtc_time *tm)
{
	unsigned int alarm_en = 0;
	
	writeb(1, RTCCON);

	if(tm->tm_year < 100 && tm->tm_year >= 0)
	{
		alarm_en |= RTCALM_YEAREN;
		writeb(bin2bcd(tm->tm_year), ALMYEAR);
	}

	if(tm->tm_mon < 13 && tm->tm_mon >= 1)
	{
		alarm_en |= RTCALM_MONEN;
		writeb(bin2bcd(tm->tm_mon), ALMMON);
	}

	if(tm->tm_mday < 32 && tm->tm_mday >= 1)
	{
		alarm_en |= RTCALM_DAYEN;
		writeb(bin2bcd(tm->tm_mday), ALMDATE);
	}

	if(tm->tm_hour < 24 && tm->tm_hour >= 0)
	{
		alarm_en |= RTCALM_HOUREN;
		writeb(bin2bcd(tm->tm_hour), ALMHOUR);
	}

	if(tm->tm_min < 60 && tm->tm_min >= 0)
	{
		alarm_en |= RTCALM_MINEN;
		writeb(bin2bcd(tm->tm_min), ALMMIN);
	}

	if(tm->tm_sec < 60 && tm->tm_sec >= 0)
	{
		alarm_en |= RTCALM_SECEN;
		writeb(bin2bcd(tm->tm_sec), ALMSEC);
	}

	if(alarm_en)
		alarm_en |= RTCALM_ALMEN;

	writeb(alarm_en, RTCALM);

	writeb(0, RTCCON);
}

void int_rtc_handle()
{
	printf("alarm ... \r\n");
}