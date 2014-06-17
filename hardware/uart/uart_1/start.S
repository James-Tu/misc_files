#include<regs-clock.h>

.globl _start
_start:
    //¹Ø±Õwatchdog
    ldr r0, =0x53000000
    ldr r1, =0x0
    str r1, [r0]

#define MPLL_400MHZ	((0x5c<<12)|(0x1<<4)|(0x1))
#define UPLL_96MHZ	((0x38<<12)|(0x2<<4)|(0x1))

	//1.	ÉèÖÃ¼Ä´æÆ÷CLKDIVN
    ldr r0, =CLKDIVN
    ldr r1, =0xd	/* FCLK:HCLK:PCLK = 1:4:8  */
    str r1, [r0]

	//2.	ÉèÖÃÒì²½×ÜÏßÄ£Ê½
	mrc  p15,0,r0,c1,c0,0
	orr  r0,r0,#0xc0000000
	mcr  p15,0,r0,c1,c0,0

    //3.	ÅäÖÃUPLL¼Ä´æÆ÷£ºUPLLCON
    ldr r0, =UPLLCON
    ldr r1, =UPLL_96MHZ
    str r1, [r0]

    //4.	ÑÓÊ±7¸ö nopÖ¸Áî
    nop
    nop
    nop
    nop
    nop
    nop
    nop

	//5.	ÅäÖÃMPLL¼Ä´æÆ÷£ºMPLLCON
    ldr r0, =MPLLCON
    ldr r1, =MPLL_400MHZ
    str r1, [r0]

	bl lowlevel_init

    //ÉèÖÃÕ»Ö¸Õë¼Ä´æÆ÷sp
    ldr sp, =0x34000000
    bl main

 mainloop:
    b mainloop
