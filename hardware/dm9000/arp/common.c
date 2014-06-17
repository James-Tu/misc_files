#include<stdio.h>
#include<regs-timer.h>
#include<io.h>

void delayms(unsigned int dly)
{
	unsigned int cnt = 1000;

	writel(cnt, TCNTB1);
	writel(readl(TCON) & ~(0xf << 8), TCON);
	//����auto reload��Update TCNTB1 & TCMPB1λΪ1����������ʱ��
	writel(readl(TCON) | (0xb << 8), TCON);
	//��� Update TCNTB1 & TCMPB1λ
	writel(readl(TCON) & ~(0x1 << 9), TCON);

	while(dly--)
	{
		while(readl(TCNTO1) >= cnt / 2); //�ȴ���TCNTO1��1000��500
		while(readl(TCNTO1) < cnt /2); //�ȴ���TCNTO1��500��1000
	}
}
