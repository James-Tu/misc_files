#include<regs-gpio.h>
#include<io.h>

int main()
{
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	while(1);

	return 0;
}
