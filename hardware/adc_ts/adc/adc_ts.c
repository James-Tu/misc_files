#include<io.h>
#include<regs-adc.h>
#include<regs-clock.h>
#include"clock.h"

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
