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
#if 0    
        GPBDAT = 0x0; //����4��led��
        n = 12 * 4096;
        while(n--);

        GPBDAT = 0xffff; //Ϩ��4��led��
        n = 12 * 4096;
        while(n--);
#endif
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
