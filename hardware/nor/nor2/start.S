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
    //�ر�watchdog
    ldr r0, =0x53000000
    ldr r1, =0x0
    str r1, [r0]

#define MPLL_400MHZ	((0x5c<<12)|(0x1<<4)|(0x1))
#define UPLL_96MHZ	((0x38<<12)|(0x2<<4)|(0x1))

	//1.	���üĴ���CLKDIVN
    ldr r0, =CLKDIVN
    ldr r1, =0xd	/* FCLK:HCLK:PCLK = 1:4:8  */
    str r1, [r0]

	//2.	�����첽����ģʽ
	mrc  p15,0,r0,c1,c0,0
	orr  r0,r0,#0xc0000000
	mcr  p15,0,r0,c1,c0,0

    //3.	����UPLL�Ĵ�����UPLLCON
    ldr r0, =UPLLCON
    ldr r1, =UPLL_96MHZ
    str r1, [r0]

    //4.	��ʱ7�� nopָ��
    nop
    nop
    nop
    nop
    nop
    nop
    nop

	//5.	����MPLL�Ĵ�����MPLLCON
    ldr r0, =MPLLCON
    ldr r1, =MPLL_400MHZ
    str r1, [r0]

	bl lowlevel_init

	//�жϴ����Ƿ��Ѿ���sdram��
	adr r0, _start	/* ȡ���_start��ǰ��λ�� */
	ldr r1, _armboot_start	/* ��������ʱ���_start��ֵ */
	cmp r0, r1	
	beq stack_setup	/* ��� r0 = r1 �� ��ʾ�����Ѿ���sdram�� */

check_boot_flash:
	ldr r0, =0x48000000
	ldr r1, [r0]	/* ��ȡ�Ĵ���BWSCON(0x48000000)��ֵ */
	ands r1, r1, #6	/* ���BWSCON[2,1]bit��ֵ��Ϊ0����ʾ��nand�����������Ϊ0����ʾ��nor����  */
	bne nor_boot
	
nand_boot:
	// TODO

nor_boot:
	adr r0, _start	/* ȡ���_start��ǰ��λ�� */
	ldr r1, _armboot_start	/* ��������ʱ���_start��ֵ */
	ldr r2, _bss_start
	sub r2, r2, r1 /* ��������size��r2 = r2 - r1  */
	add r2, r0, r2
	
copy_loop:
	//�������뵽sdram��
	ldmia r0!, {r3-r10}
	stmia r1!, {r3-r10}
	cmp r0, r2
	ble copy_loop

stack_setup:
    //����irqջָ��Ĵ���sp
    msr cpsr_c, #0xd2
    ldr sp, =0x34000000

	//����svcջָ��Ĵ���sp
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