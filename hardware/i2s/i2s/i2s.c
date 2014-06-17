#include<io.h>
#include<regs-gpio.h>
#include<regs-i2s.h>
#include<regs-clock.h>
#include"common.h"
#include"clock.h"

/*
	L3MODE  GPB2
	L3DATA  GPB3
	L3CLOCK GPB4
*/
#define L3MODE_H()		writel(readl(GPBDAT) | (1 << 2), GPBDAT)
#define L3MODE_L()		writel(readl(GPBDAT) & ~(1 << 2), GPBDAT)
#define L3DATA_H()		writel(readl(GPBDAT) | (1 << 3), GPBDAT)
#define L3DATA_L()		writel(readl(GPBDAT) & ~(1 << 3), GPBDAT)
#define L3CLOCK_H()		writel(readl(GPBDAT) | (1 << 4), GPBDAT)
#define L3CLOCK_L()		writel(readl(GPBDAT) & ~(1 << 4), GPBDAT)
#define l3_delay()		delayms(1)

#define UDA1341_L3ADDR		5
#define UDA1341_DATA0_ADDR		((UDA1341_L3ADDR << 2) | 0)
#define UDA1341_DATA1_ADDR		((UDA1341_L3ADDR << 2) | 1)
#define UDA1341_STATUS_ADDR		((UDA1341_L3ADDR << 2) | 2)

#define UDA1341_EXT_ADDR(a)		(0xc0 | (a))
#define UDA1341_EXT_DATA(d)		(0xe0 | (d))

void i2s_init()
{
	//i2s接口初始化
	writel((readl(GPECON) & ~0x3ff) | 0x2aa, GPECON); /* 设置GPE0-4功能为i2s功能 */
	writel(readl(GPEUP) | 0x1f, GPEUP);  /* GPE0-4上啦无效 */

	clk_enable(CLKCON_IIS, 1);

	//L3接口初始化
	writel((readl(GPBCON) & ~(0x3f << 4)) | (0x15 << 4), GPBCON); /* 设置GPB2,3,4为输出 */
	writel(readl(GPBUP) | (0x7 << 2), GPBUP); /* GPB2,3,4上啦无效 */

	L3MODE_H();
	L3CLOCK_H();
}

void play_audio(unsigned char *buf, int len)
{
	int i, n;
	unsigned short* b = (unsigned short*)buf;
	
	writel((1 << 2) | (1 << 1), IISCON); //tx Not idle, rx Idle, IIS prescaler enable
	//tx mode, 每个声道16bit，codec时钟 384fs， SCLK时钟 32fs
	writel((2 << 6) | (1 << 3) | (1 << 2) | 1, IISMOD);

	/*
		设置 fs = 44.1KHZ   codec时钟=384fs=16.9344MHZ
		codec时钟 = PCLK / IPSR_B,     IPSR_B = 3
	*/
	writel((2 << 5) | 2, IISPSR);
	writel(1 << 13, IISFCON); //tx fifo enable

	writel(readl(IISCON) | 0x01, IISCON); //enable i2s

	while(len > 0)
	{
		while(readl(IISCON) & (1 << 7)); /* 等待，直到tx fifo为空 */

		n = (len > 64)? 64 : len;
		for(i = 0; i < n / 2; i++)
			writew(*b++, IISFIFO); /* 向fifo中写数据 */

		len -= n;
	}
	
	writel(0, IISCON); // disable i2s
}

void record_audio(unsigned char *buf, int len)
{
	int i, n;
	unsigned short* b = (unsigned short*)buf;
	
	writel((1 << 3) | (1 << 1), IISCON); //rx Not idle, tx Idle, IIS prescaler enable
	//rx mode, 每个声道16bit，codec时钟 384fs， SCLK时钟 32fs
	writel((1 << 6) | (1 << 3) | (1 << 2) | 1, IISMOD);

	/*
		设置 fs = 44.1KHZ   codec时钟=384fs=16.9344MHZ
		codec时钟 = PCLK / IPSR_B,     IPSR_B = 3
	*/
	writel((2 << 5) | 2, IISPSR);
	writel(1 << 12, IISFCON); //rx fifo enable

	writel(readl(IISCON) | 0x01, IISCON); //enable i2s

	while(len > 0)
	{
		while(readl(IISCON) & (1 << 6)); /* 等待，直到rx fifo为满 */

		n = (len > 64)? 64 : len;
		for(i = 0; i < n / 2; i++)
			*b++ = readw(IISFIFO); /* 从fifo中读取数据 */

		len -= n;
	}
	
	writel(0, IISCON); // disable i2s
}

/*
	flag = 1: 地址模式
	flag = 0: 数据模式
*/
void l3_write(unsigned char v, int flag)
{
	int i;
	
	if(flag)
		L3MODE_L();

	l3_delay();

	for(i = 0; i < 8; i++)
	{
		L3CLOCK_L();
		if(v & 0x1)
			L3DATA_H();
		else
			L3DATA_L();
			
		l3_delay();
		L3CLOCK_H();
		l3_delay();
		v = v >> 1;
	}

	L3CLOCK_H();
	L3MODE_H();
}

void l3_write_addr(unsigned char addr)
{
	l3_write(addr, 1);
}

void l3_write_data(unsigned char data)
{
	l3_write(data, 0);
}

void uda1341_init()
{
	l3_write_addr(UDA1341_STATUS_ADDR);
	l3_write_data((1 << 6) | (1 << 4) | 1); // reset, 384fs, I2S-bus, DC-filtering
	delayms(1);
	l3_write_data((1 << 4) | 1); //384fs, I2S-bus, DC-filtering
	l3_write_data((1 << 7) | 3); //ADC on, DAC on

	//选择录用通道2
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data(UDA1341_EXT_ADDR(2));
	l3_write_data(UDA1341_EXT_DATA(2));
}

/*
	1 <= volume <= 62
	1:  音量最小
	62: 音量最大
*/
void uda1341_set_volume(int volume)
{
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data(63 - volume);
}

//mute=0 不静音           mute=1 静音
void uda1341_set_mute(int mute)
{
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data((1 << 7) | (mute << 2));
}