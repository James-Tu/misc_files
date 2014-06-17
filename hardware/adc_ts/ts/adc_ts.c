#include<io.h>
#include<stdio.h>
#include<regs-adc.h>
#include<regs-clock.h>
#include<regs-irq.h>
#include"clock.h"
#include"common.h"

//触摸屏接口模式
#define TS_MODE_AUTO_XY		0xc
#define TS_MODE_WAIT_INT(up_down)	(0xd3 | ((up_down) << 8))
#define STYLUS_DOWN 0
#define STYLUS_UP 1

//A/D converter freq. = 50MHz/(49+1) = 1MHz
#define PRSCEN (1 << 14)
#define PRSCVL (49 << 6)

void adc_ts_init()
{
	clk_enable(CLKCON_ADC, 1);
}

unsigned int read_adc(int ch)
{
	//设置ADC的时钟频率，选择输入通道，ch取值:0,1,2,3
	writel(PRSCEN | PRSCVL | (ch << 3), ADCCON);
	//启动AD转换
	writel(readl(ADCCON) | 0x1, ADCCON);
	//等待，直到AD转换真正开始
	while(readl(ADCCON) & 0x1);
	//等待，直到AD转换完成
	while(!(readl(ADCCON) & (1 << 15)));

	return readl(ADCDAT0) & 0x3ff;
}

void ts_init()
{
	//设置ADC的时钟频率
	writel(PRSCEN | PRSCVL, ADCCON);
	//采样延时: (1 / 3.68M) * 50000 = 13.58ms
	writel(50000, ADCDLY);
	//进入等待中断模式，等待触摸屏被按下
	writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
}

void sub_int_tc()
{
	printf("sub_int_tc\r\n");

	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_tc: Stylus up state\r\n");
		//进入等待中断模式，等待触摸屏被按下
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
	}
	else
	{
		printf("sub_int_tc: Stylus down state\r\n");
		//进入自动连续的X，Y坐标转换模式
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//启动AD转换
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//清除中断:IRQ_SUB_TC
	writel(1 << IRQ_SUB_TC_OFT, SUBSRCPND);
}

void sub_int_adc()
{
	int x, y;
	
	printf("sub_int_adc\r\n");

	x = readl(ADCDAT0) & 0x3ff;
	y = readl(ADCDAT1) & 0x3ff;
	printf("sub_int_adc: [%d,%d]\r\n", x, y);

	//进入等待中断模式，等待触摸屏被释放
	writel(TS_MODE_WAIT_INT(STYLUS_UP), ADCTSC);

	delayms(20);
	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_adc: Stylus up state\r\n");
		//进入等待中断模式，等待触摸屏被按下
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
	}
	else
	{
		printf("sub_int_adc: Stylus down state\r\n");
		//进入自动连续的X，Y坐标转换模式
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//启动AD转换
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//清除中断:IRQ_SUB_ADC
	writel(1 << IRQ_SUB_ADC_S_OFT, SUBSRCPND);
}

void int_adc_handle()
{
	printf("int_adc_handle\r\n");
	
	if(readl(SUBSRCPND) & (1 << IRQ_SUB_TC_OFT))
		sub_int_tc();

	if(readl(SUBSRCPND) & (1 << IRQ_SUB_ADC_S_OFT))
		sub_int_adc();
}
