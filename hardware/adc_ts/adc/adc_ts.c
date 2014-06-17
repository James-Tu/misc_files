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
	//����ADC��ʱ��Ƶ�ʣ�ѡ������ͨ����chȡֵ:0,1,2,3
	writel(PRSCEN | PRSCVL | (ch << 3), ADCCON);
	//����ADת��
	writel(readl(ADCCON) | 0x1, ADCCON);
	//�ȴ���ֱ��ADת��������ʼ
	while(readl(ADCCON) & 0x1);
	//�ȴ���ֱ��ADת�����
	while(!(readl(ADCCON) & (1 << 15)));

	return readl(ADCDAT0) & 0x3ff;
}
