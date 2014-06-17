#include<stdio.h>
#include<regs-clock.h>
#include<regs-timer.h>
#include<regs-gpio.h>
#include<io.h>
#include"clock.h"
#include"led.h"

void timer_init()
{
	clk_enable(CLKCON_PWMT, 1);

	/*
		Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
		{prescaler value} = 0~255
		{divider value} = 2, 4, 8, 16
		timer0,1 ����ʱ��: 1MHZ =  50MHZ / (24 + 1) / 2 		���ڣ�1us
		timer2,3,4����ʱ��: 50KHZ =  50MHZ / (249 + 1) / 4		���ڣ�20us

	*/
	writel(24 | (249 << 8), TCFG0);
	writel(0x11100, TCFG1);	
}

void timer4_init()
{
	//��ʱ1s
	writel(50000, TCNTB4);

	writel(readl(TCON) & ~(0x7 << 20), TCON);
	//����auto reload��Update TCNTB4Ϊ1����������ʱ��
	writel(readl(TCON) | (0x7 << 20), TCON);
	//���Update TCNTB4λ
	writel(readl(TCON) & ~(0x1 << 21), TCON);
}

void int_timer4_handle()
{
	static unsigned int cnt = 0;

	printf("int_timer4_handle\r\n");

	cnt++;
	if(cnt % 2)
		led_on(0xf);
	else
		led_on(0x0);
}

void timer0_init()
{
	//����GPB0Ϊ TOUT0����
	writel((readl(GPBCON) & ~0x3) | 0x2, GPBCON);
	//pwm ʱ������20ms
	writel(20000, TCNTB0);
	writel(10000, TCMPB0);

	writel(readl(TCON) & ~0xf, TCON);
	//����auto reload��Update TCNTB0 & TCMPB0Ϊ1��Inverter off����������ʱ��
	writel(0xb, TCON);
	//���Timer 0 manual updateλ
	writel(readl(TCON) & ~(0x1 << 1), TCON);
}