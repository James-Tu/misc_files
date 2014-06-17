#include<stdio.h>
#include<regs-timer.h>
#include<io.h>

void delayms(unsigned int dly)
{
	unsigned int cnt = 1000;

	writel(cnt, TCNTB1);
	writel(readl(TCON) & ~(0xf << 8), TCON);
	//设置auto reload，Update TCNTB1 & TCMPB1位为1，并启动定时器
	writel(readl(TCON) | (0xb << 8), TCON);
	//清除 Update TCNTB1 & TCMPB1位
	writel(readl(TCON) & ~(0x1 << 9), TCON);

	while(dly--)
	{
		while(readl(TCNTO1) >= cnt / 2); //等待，TCNTO1从1000到500
		while(readl(TCNTO1) < cnt /2); //等待，TCNTO1从500到1000
	}
}
