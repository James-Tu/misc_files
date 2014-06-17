#include<regs-clock.h>

.globl _start
_start:
	b start_code
	b .
	b .
	b .
	b .
	b .
	ldr pc, _irq	//irq
	b .		//fiq

_irq:
	.word irq

_armboot_start:
	.word _start

.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

start_code:
    //关闭watchdog
    ldr r0, =0x53000000
    ldr r1, =0x0
    str r1, [r0]

#define MPLL_400MHZ	((0x5c<<12)|(0x1<<4)|(0x1))
#define UPLL_96MHZ	((0x38<<12)|(0x2<<4)|(0x1))

	//1.	设置寄存器CLKDIVN
    ldr r0, =CLKDIVN
    ldr r1, =0xd	/* FCLK:HCLK:PCLK = 1:4:8  */
    str r1, [r0]

	//2.	设置异步总线模式
	mrc  p15,0,r0,c1,c0,0
	orr  r0,r0,#0xc0000000
	mcr  p15,0,r0,c1,c0,0

    //3.	配置UPLL寄存器：UPLLCON
    ldr r0, =UPLLCON
    ldr r1, =UPLL_96MHZ
    str r1, [r0]

    //4.	延时7个 nop指令
    nop
    nop
    nop
    nop
    nop
    nop
    nop

	//5.	配置MPLL寄存器：MPLLCON
    ldr r0, =MPLLCON
    ldr r1, =MPLL_400MHZ
    str r1, [r0]

	bl lowlevel_init

	//判断代码是否已经在sdram中
	adr r0, _start	/* 取标号_start当前的位置 */
	ldr r1, _armboot_start	/* 程序连接时标号_start的值 */
	cmp r0, r1	
	beq stack_setup	/* 如果 r0 = r1 ， 表示代码已经在sdram中 */

check_boot_flash:
	ldr r0, =0x48000000
	ldr r1, [r0]	/* 读取寄存器BWSCON(0x48000000)的值 */
	ands r1, r1, #6	/* 如果BWSCON[2,1]bit的值都为0，表示从nand启动，如果不为0，表示从nor启动  */
	bne nor_boot
	
nand_boot:
	// TODO

nor_boot:
	adr r0, _start	/* 取标号_start当前的位置 */
	ldr r1, _armboot_start	/* 程序连接时标号_start的值 */
	ldr r2, _bss_start
	sub r2, r2, r1 /* 计算代码的size，r2 = r2 - r1  */
	add r2, r0, r2
	
copy_loop:
	//拷贝代码到sdram中
	ldmia r0!, {r3-r10}
	stmia r1!, {r3-r10}
	cmp r0, r2
	ble copy_loop

stack_setup:
    //设置irq栈指针寄存器sp
    msr cpsr_c, #0xd2
    ldr sp, =0x34000000

	//设置svc栈指针寄存器sp
	msr cpsr_c, #0xd3
	ldr sp, =(0x34000000 - 0x100000)

	//enable irq
	msr cpsr_c, #0x53

clear_bss:
	ldr r0, _bss_start
	ldr r1, _bss_end
	mov r2, #0x0
	
clbss_1:
	str r2, [r0]
	add r0, r0, #4
	cmp r0, r1
	ble clbss_1

	ldr pc, _main
_main:	
    .word main

 mainloop:
    b mainloop

irq:
	sub lr, lr, #4
	stmdb sp!, {r0-r12, lr}
	bl irq_handle
	ldmia sp!, {r0-r12, pc} ^
