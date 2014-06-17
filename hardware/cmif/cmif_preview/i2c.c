#include<io.h>
#include<stdio.h>
#include<regs-clock.h>
#include<regs-gpio.h>
#include<regs-i2c.h>
#include"clock.h"
#include"common.h"

void i2c_init()
{
	//设置GPE14,15 pin脚功能为: IICSCL, IICSDA
	writel((readl(GPECON) & ~(0xf << 28)) | (0xa << 28), GPECON);
	clk_enable(CLKCON_IIC, 1);

	//IICCLK = fPCLK /512
	//Tx clock = IICCLK/(IICCON[3:0]+1) = (50MHZ / 512) / (0 + 1) = 97.65625KHZ
	writeb((1 << 7) | (1 << 6) | (1 << 5), IICCON);
	writeb(1 << 4, IICSTAT);
}

int i2c_write(unsigned char slave_addr, unsigned char *buf, int len)
{
	int n = len;
	
	writeb((3 << 6) | (1 << 4), IICSTAT); //Master transmit mode
	writeb(slave_addr << 1, IICDS);	// 写从机地址
	writeb(0xf0, IICSTAT); //Master transmit mode, START signal generation

	while(1)
	{
		while(!(readb(IICCON) & (1 << 4)));		/* 等待中断的发生 */

		if(!len)
		{
			writeb(0xd0, IICSTAT); //Master transmit mode, STOP signal generation
			writeb(readb(IICCON) & ~(1 << 4), IICCON); //Clear pending condition, Resume the operation
			delayms(10);
			break;
		}

		len--;
		writeb(*buf++, IICDS);
		writeb(readb(IICCON) & ~(1 << 4), IICCON); //Clear pending condition, Resume the operation
	}

	return n - len;
}

int i2c_read(unsigned char slave_addr, unsigned char *buf, int len)
{
	int n = len;
	int flag = 1;
	
	writeb((0x2 << 6) | (1 << 4), IICSTAT); //Master receive mode
	writeb((slave_addr << 1) | 0x1, IICDS);	// 写从机地址
	writeb(0xb0, IICSTAT); //Master receive mode, START signal generation

	while(1)
	{
		while(!(readb(IICCON) & (1 << 4)));		/* 等待中断的发生 */

		if(!len)
		{
			writeb(0x90, IICSTAT); //Master receive mode, STOP signal generation
			writeb(readb(IICCON) & ~(1 << 4), IICCON); //Clear pending condition, Resume the operation
			delayms(10);
			break;
		}

		if(flag)
			flag = 0;
		else
		{
			len--;
			*buf++ = readb(IICDS);
		}

		if(1 == len)
			writeb(readb(IICCON) & ~((1 << 4) | (1 << 7)), IICCON); //ack disable, Clear pending condition, Resume the operation
		else
			writeb(readb(IICCON) & ~(1 << 4), IICCON); //Clear pending condition, Resume the operation
	}

	writeb(readb(IICCON) | (1 << 7), IICCON); // ack enable
	return n - len;
}

