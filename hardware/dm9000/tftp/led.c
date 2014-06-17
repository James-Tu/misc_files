#include <io.h>
#include <regs-gpio.h>

/*
	点亮led灯
	on 低4bit位，每一个bit对应一个led灯
	bit0 --- led1
	bit1 --- led2
	bit2 --- led3
	bit3 --- led4
	如果对应的bit=0   熄灭led灯
	如果对应的bit=1   点亮led灯
*/
void led_on(unsigned int on)
{
	unsigned int temp;

	temp = readl(GPBCON);
	temp = (temp & ~(0xff << 10)) | (0x55 << 10);
	writel(temp, GPBCON);

	on = ~on;
	temp = readl(GPBDAT);
	temp = (temp & ~(0xf << 5)) | ((on & 0xf) << 5);
	writel(temp, GPBDAT);
}
