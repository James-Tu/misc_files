#include<regs-serial.h>
#include<regs-gpio.h>
#include<regs-clock.h>
#include<map.h>
#include<io.h>

void uart0_init()
{
	unsigned int uart_base, temp;

	uart_base = S3C2440_PA_UART0;

	//����pin��GPH2,3ΪTXD,RXD����
	temp = readl(GPHCON);
	temp = (temp & ~0xf0) | 0xa0;
	writel(temp, GPHCON);
	writel(readl(GPHUP) | 0xc, GPHUP);
	
	clk_enable(CLKCON_UART0, 1);

	writel(0x3, uart_base + ULCON); //8λ����λ��1λֹͣλ������żУ��
	writel(0x5, uart_base + UCON); // clock src: PCLK
	writel(0x0, uart_base + UFCON); // non-fifo
	writel(0x0, uart_base + UMCON); //������
	//UBRDIVn  = (int)( UART clock / ( buad rate x 16) ) �C1
	//UBRDIVn  = (int)( 50 * 1000000  / ( 115200 x 16) ) �C1 = 26
	writel(26, uart_base + UBRDIV); //������: 115200
}

void putc(unsigned char c)
{
	unsigned int uart_base = S3C2440_PA_UART0;

	//�ȴ���ֱ��transmit buffer��transmit shiftΪ��
	while(!(readl(uart_base + UTRSTAT) & (1 << 2)));

	writeb(c, uart_base + UTXH);
}

unsigned char getc()
{
	unsigned int uart_base = S3C2440_PA_UART0;
	unsigned char ret;
	
	while(!(readl(uart_base + UTRSTAT) & 0x1));
	ret = readb(uart_base + URXH);

	//0xd �س�
	//0xa ����
	if(0xd == ret || 0xa == ret)
	{
		putc(0xd);
		putc(0xa);
	}
	else
		putc(ret);
	
	return ret;
}

