#include<regs-gpio.h>
#include<io.h>

int main()
{
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();
	while(1)
	{
		led_on(0xf);
		delayms(500);
		led_on(0x0);
		delayms(500);
	}

	return 0;
}
