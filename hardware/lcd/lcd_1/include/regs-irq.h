#ifndef __REGS_IRQ_H
#define __REGS_IRQ_H

#define	SRCPND 0X4A000000
#define	INTMOD 0X4A000004
#define	INTMSK 0X4A000008
#define	PRIORITY 0X4A00000C
#define	INTPND 0X4A000010
#define	INTOFFSET 0X4A000014
#define	SUBSRCPND 0X4A000018
#define	INTSUBMSK 0X4A00001C

#define IRQ_EINT0_OFT     0
#define IRQ_EINT1_OFT     1
#define IRQ_EINT2_OFT     2
#define IRQ_EINT3_OFT     3
#define IRQ_EINT4_7_OFT   4
#define IRQ_EINT8_23_OFT  5
#define IRQ_NOTUSED6_OFT  6
#define IRQ_BAT_FLT_OFT   7
#define IRQ_TICK_OFT      8
#define IRQ_WDT_OFT       9
#define IRQ_TIMER0_OFT    10
#define IRQ_TIMER1_OFT    11
#define IRQ_TIMER2_OFT    12
#define IRQ_TIMER3_OFT    13
#define IRQ_TIMER4_OFT    14
#define IRQ_UART2_OFT     15
#define IRQ_LCD_OFT       16
#define IRQ_DMA0_OFT      17
#define IRQ_DMA1_OFT      18
#define IRQ_DMA2_OFT      19
#define IRQ_DMA3_OFT      20
#define IRQ_SDI_OFT       21
#define IRQ_SPI0_OFT      22
#define IRQ_UART1_OFT     23
#define IRQ_NOTUSED24_OFT 24
#define IRQ_USBD_OFT      25
#define IRQ_USBH_OFT      26
#define IRQ_IIC_OFT       27
#define IRQ_UART0_OFT     28
#define IRQ_SPI1_OFT      29
#define IRQ_RTC_OFT       30
#define IRQ_ADC_OFT       31

#endif
