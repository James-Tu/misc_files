#include<io.h>
#include<regs-watchdog.h>

void watchdog_init()
{
	//watchdogʱ�� = PCLK / (Prescaler value + 1) / Division_factor = 50MHZ / 125 / 16 = 25KHZ ���ڣ�40us
	writel((124 << 8) | 1, WTCON);

	//��ʱ2s:  2s / 40us = 50000
	writel(50000, WTDAT);
	writel(50000, WTCNT);

	//ʹ��watchdog
	writel(readl(WTCON) | (1 << 5), WTCON);
}

void watchdog_set_cnt(unsigned int cnt)
{
	writel(cnt, WTCNT);
}