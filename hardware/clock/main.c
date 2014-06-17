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
        led_on(1 << 0); //点亮第1个led灯
		delayms(1000);


        led_on(1 << 1); //点亮第2个led灯
		delayms(1000);


        led_on(1 << 2); //点亮第3个led灯
		delayms(1000);


        led_on(1 << 3); //点亮第4个led灯
		delayms(1000);

    }
}
