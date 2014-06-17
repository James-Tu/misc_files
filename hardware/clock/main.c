#include <io.h>
#include <regs-gpio.h>

//#define GPBCON (*((volatile unsigned int*)0x56000010))
//#define GPBDAT (*((volatile unsigned int*)0x56000014))

void main()
{
    volatile unsigned int n;
    
//    GPBCON = 0x15400;
	writel(0x15400, GPBCON);

    while(1)
    {
        led_on(1 << 0); //������1��led��
		delayms(1000);


        led_on(1 << 1); //������2��led��
		delayms(1000);


        led_on(1 << 2); //������3��led��
		delayms(1000);


        led_on(1 << 3); //������4��led��
		delayms(1000);

    }
}
