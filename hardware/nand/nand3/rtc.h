#ifndef __RTC_H
#define __RTC_H

struct rtc_time
{
	int tm_year;
	int tm_mon;
	int tm_mday;
	int tm_hour;
	int tm_min;
	int tm_sec;
	int tm_wday;
};

static inline unsigned char bcd2bin(unsigned char val)
{
	return (val & 0xf) + (val >> 4) * 10;
}

static inline unsigned char bin2bcd(unsigned char val)
{
	return ((val / 10) << 4) + val % 10;
}

extern void rtc_init();
extern void rtc_settime(struct rtc_time *tm);
extern void rtc_gettime(struct rtc_time *tm);
extern void rtc_setalarm(struct rtc_time *tm);
extern void int_rtc_handle();

#endif
