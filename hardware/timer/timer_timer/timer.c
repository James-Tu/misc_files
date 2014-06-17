#include<stdio.h>
#include<regs-clock.h>
#include<regs-timer.h>
#include<io.h>

void timer_init()
{
	clk_enable(CLKCON_PWMT, 1);

	/*
		Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
		{prescaler value} = 0~255
		{divider value} = 2, 4, 8, 16
		timer0,1 输入时钟: 1MHZ =  50MHZ / (24 + 1) / 2 		周期：1us
		timer2,3,4输入时钟: 50KHZ =  50MHZ / (249 + 1) / 4		周期：20us

	*/
	writel(24 | (249 << 8), TCFG0);
	writel(0x11100, TCFG1);	
}

void timer4_init()
{
	//定时1s
	writel(50000, TCNTB4);

	writel(readl(TCON) & ~(0x7 << 20), TCON);
	//设置auto reload，Update TCNTB4为1，并启动定时器
	writel(readl(TCON) | (0x7 << 20), TCON);
	//清除Update TCNTB4位
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