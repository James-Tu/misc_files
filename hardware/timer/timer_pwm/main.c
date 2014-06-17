#include<regs-gpio.h>
#include<regs-timer.h>
#include<io.h>
#include<stdio.h>
#include"irq.h"
#include"timer.h"
#include"serial.h"
#include"button.h"

int main()
{
	int cnt = 0;
	char c;
	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();

	timer0_init();

	while(1)
	{
		c = getc();

		if(c == '+')
		{
			cnt = cnt + 1000;
			if(cnt >= 20000)
				cnt = 19999;

			printf(" cnt = %d\r\n", cnt);
			writel(cnt, TCMPB0);
		}
		
		if(c == '-')
		{
			cnt = cnt - 1000;
			if(cnt <= 0)
				cnt = 1;
			printf(" cnt = %d\r\n", cnt);
			writel(cnt, TCMPB0);
		}
	}

	return 0;
}
