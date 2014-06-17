#include<io.h>
#include<regs-clock.h>

void clk_enable(unsigned int clocks, unsigned int enable)
{
	unsigned int clkcon;

	clkcon = readl(CLKCON);
	clkcon &= ~clocks;

	if(enable)
		clkcon |= clocks;

	writel(clkcon, CLKCON);
}
