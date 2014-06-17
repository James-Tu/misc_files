#include<regs-serial.h>
#include<regs-gpio.h>
#include<regs-clock.h>
#include<map.h>
#include<io.h>

void uart0_init()
{
	unsigned int uart_base, temp;

	uart_base = S3C2440_PA_UART0;

	//配置pin脚GPH2,3为TXD,RXD功能
	temp = readl(GPHCON);
	temp = (temp & ~0xf0) | 0xa0;
	writel(temp, GPHCON);
	writel(readl(GPHUP) | 0xc, GPHUP);
	
	clk_enable(CLKCON_UART0, 1);

	writel(0x3, uart_base + ULCON); //8位数据位，1位停止位，无奇偶校验
	writel(0x5, uart_base + UCON); // clock src: PCLK
	writel(0x0, uart_base + UFCON); // non-fifo
	writel(0x0, uart_base + UMCON); //无流控
	//UBRDIVn  = (int)( UART clock / ( buad rate x 16) ) –1
	//UBRDIVn  = (int)( 50 * 1000000  / ( 115200 x 16) ) –1 = 26
	writel(26, uart_base + UBRDIV); //波特率: 115200
}

void putc(unsigned char c)
{
	unsigned int uart_base = S3C2440_PA_UART0;

	//等待，直到transmit buffer和transmit shift为空
	while(!(readl(uart_base + UTRSTAT) & (1 << 2)));

	writeb(c, uart_base + UTXH);
}

unsigned char getc()
{
	unsigned int uart_base = S3C2440_PA_UART0;
	unsigned char ret;
	
	while(!(readl(uart_base + UTRSTAT) & 0x1));
	ret = readb(uart_base + URXH);

	//0xd 回车
	//0xa 换行
	if(0xd == ret || 0xa == ret)
	{
		putc(0xd);
		putc(0xa);
	}
	else
		putc(ret);
	
	return ret;
}

