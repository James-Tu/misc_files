#include<io.h>
#include<stdio.h>
#include<regs-adc.h>
#include<regs-clock.h>
#include<regs-irq.h>
#include"clock.h"
#include"common.h"

//�������ӿ�ģʽ
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

void ts_init()
{
	//����ADC��ʱ��Ƶ��
	writel(PRSCEN | PRSCVL, ADCCON);
	//������ʱ: (1 / 3.68M) * 50000 = 13.58ms
	writel(50000, ADCDLY);
	//����ȴ��ж�ģʽ���ȴ�������������
	writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
}

void sub_int_tc()
{
	printf("sub_int_tc\r\n");

	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_tc: Stylus up state\r\n");
		//����ȴ��ж�ģʽ���ȴ�������������
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
	}
	else
	{
		printf("sub_int_tc: Stylus down state\r\n");
		//�����Զ�������X��Y����ת��ģʽ
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//����ADת��
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//����ж�:IRQ_SUB_TC
	writel(1 << IRQ_SUB_TC_OFT, SUBSRCPND);
}

void sub_int_adc()
{
	int x, y;
	
	printf("sub_int_adc\r\n");

	x = readl(ADCDAT0) & 0x3ff;
	y = readl(ADCDAT1) & 0x3ff;
	printf("sub_int_adc: [%d,%d]\r\n", x, y);

	//����ȴ��ж�ģʽ���ȴ����������ͷ�
	writel(TS_MODE_WAIT_INT(STYLUS_UP), ADCTSC);

	delayms(20);
	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_adc: Stylus up state\r\n");
		//����ȴ��ж�ģʽ���ȴ�������������
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
	}
	else
	{
		printf("sub_int_adc: Stylus down state\r\n");
		//�����Զ�������X��Y����ת��ģʽ
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//����ADת��
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//����ж�:IRQ_SUB_ADC
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
