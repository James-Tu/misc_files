#include<io.h>
#include<regs-gpio.h>
#include"common.h"

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
	//L3接口初始化
	writel((readl(GPBCON) & ~(0x3f << 4)) | (0x15 << 4), GPBCON); /* 设置GPB2,3,4为输出 */
	writel(readl(GPBUP) | (0x7 << 2), GPBUP); /* GPB2,3,4上啦无效 */

	L3MODE_H();
	L3CLOCK_H();
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