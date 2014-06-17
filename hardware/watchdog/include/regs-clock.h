#ifndef __REGS_CLOCK_H
#define __REGS_CLOCK_H

#define	LOCKTIME 0x4C000000
#define	MPLLCON 0x4C000004
#define	UPLLCON 0x4C000008
#define	CLKCON 0x4C00000C
#define	CLKSLOW 0x4C000010
#define	CLKDIVN 0x4C000014
#define	CAMDIVN 0x4C000018

//clkcon bit 2 -- bit 20, other bits reserved
#define CLKCON_IDLE	     (1<<2)
#define CLKCON_POWER     (1<<3)
#define CLKCON_NAND	     (1<<4)
#define CLKCON_LCDC	     (1<<5)
#define CLKCON_USBH	     (1<<6)
#define CLKCON_USBD	     (1<<7)
#define CLKCON_PWMT	     (1<<8)
#define CLKCON_SDI	     (1<<9)
#define CLKCON_UART0     (1<<10)
#define CLKCON_UART1     (1<<11)
#define CLKCON_UART2     (1<<12)
#define CLKCON_GPIO	     (1<<13)
#define CLKCON_RTC	     (1<<14)
#define CLKCON_ADC	     (1<<15)
#define CLKCON_IIC	     (1<<16)
#define CLKCON_IIS	     (1<<17)
#define CLKCON_SPI	     (1<<18)
#define CLKCON_CAMERA    (1<<19)
#define CLKCON_AC97      (1<<20)

#endif
