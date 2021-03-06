#include<regs-gpio.h>
#include<regs-irq.h>
#include<io.h>
#include<stdio.h>

extern void eint8_23_handle();
extern void int_timer4_handle();
void (*irq_handle_arr[32])(void);

void copy_vec()
{
	int i;
	unsigned int *src = (unsigned int *)0x30000000;
	unsigned int *dst = (unsigned int *)0x0;

	for(i = 0; i < 64; i++)
		*dst++ = *src++;
}

void dummy_irq()
{
	printf("dummy_irq\r\n");
	while(1);
}

void irq_init()
{
	unsigned int temp;

	for(temp = 0; temp < 32; temp++)
		irq_handle_arr[temp] = dummy_irq;

	//enable irq EINT8_23
	temp = readl(INTMSK);
	temp = temp & ~(1 << IRQ_EINT8_23_OFT);
	temp = temp & ~(1 << IRQ_TIMER4_OFT); //enable irq TIMER4
	writel(temp, INTMSK);

	irq_handle_arr[IRQ_EINT8_23_OFT] = eint8_23_handle;
	irq_handle_arr[IRQ_TIMER4_OFT] = int_timer4_handle;
}

void irq_handle()
{
	unsigned int offset = readl(INTOFFSET);

	//调用中断处理函数
	irq_handle_arr[offset]();

	//清除中断
	writel(1 << offset, SRCPND);
	writel(readl(INTPND), INTPND);
}
