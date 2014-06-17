#include<regs-gpio.h>
#include<io.h>

int main()
{
	unsigned int temp, on;

	//设置GPG0,3,5,6为输入功能
	temp = readl(GPGCON);
	temp = temp & ~((0x3) | (0x3 << 6) | (0xf << 10));
	writel(temp, GPGCON);

	while(1)
	{
		on = 0;
		temp = readl(GPGDAT);

		if(temp & 0x01)		//button 1
		{}
		else
			on |= 0x01;

		if(temp & (1 << 3))		//button 2
		{}
		else
			on |= 0x02;

		if(temp & (1 << 5)) 	//button 3
		{}
		else
			on |= 0x04;

		if(temp & (1 << 6)) 	//button 4
		{}
		else
			on |= 0x08;

		led_on(on);
	}

	return 0;
}
