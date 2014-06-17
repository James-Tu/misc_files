#include <io.h>
#include <regs-gpio.h>

/*
	����led��
	on ��4bitλ��ÿһ��bit��Ӧһ��led��
	bit0 --- led1
	bit1 --- led2
	bit2 --- led3
	bit3 --- led4
	�����Ӧ��bit=0   Ϩ��led��
	�����Ӧ��bit=1   ����led��
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
