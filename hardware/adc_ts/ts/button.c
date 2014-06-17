#include<regs-gpio.h>
#include<io.h>
#include<stdio.h>


void button_init()
{
	unsigned int temp;

	//设置button pin脚功能为EINT
	temp = readl(GPGCON);
	temp = temp & ~(0x3 | (0x3 << 6) | (0x3f << 10) | (0x3 << 22));
	temp = temp | (0x2 | (0x2 << 6) | (0x2a << 10) | (0x2 << 22));
	writel(temp, GPGCON);

	//设置button对应的EINT下降沿触发
	temp = readl(EXTINT1);
	temp = temp & ~(0xf | (0xf << 12) | (0xfff << 20));
	temp = temp | (0x2 | (0x2 << 12) | (0x222 << 20));
	writel(temp, EXTINT1);
	temp = readl(EXTINT2);
	temp = (temp & ~(0xf << 12)) | (0x2 << 12);
	writel(temp, EXTINT2);

	//使能中断EINT:8,11,13,14,15,19
	temp = readl(EINTMASK);
	temp = temp & ~((1 << 19) | (7 << 13) | (1 << 11) | (1 << 8));
	writel(temp, EINTMASK);
}

void eint8_23_handle()
{
	unsigned int temp;

	temp = readl(EINTPEND);

	if(temp & (1 << 8))
		printf("button 1 \r\n");

	if(temp & (1 << 11))
		printf("button 2 \r\n");

	if(temp & (1 << 13))
		printf("button 3 \r\n");

	if(temp & (1 << 14))
		printf("button 4 \r\n");

	if(temp & (1 << 15))
		printf("button 5 \r\n");

	if(temp & (1 << 19))
		printf("button 6 \r\n");

	writel(temp, EINTPEND);
}
