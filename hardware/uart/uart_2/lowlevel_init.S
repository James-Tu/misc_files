#define BWSCON 0x48000000

.globl lowlevel_init
lowlevel_init:
	ldr r0, =BWSCON		/* r0 = 0x48000000 */
	adr r1, mem_cfg		/* r1 = mem_cfg */
	add r2, r0, #52		/* r2 = 0x48000000 +  52    52 = 13 * 4 */
1:
	ldr r3, [r1], #4  /* r3 = [r1], r1 = r1 + 4 */
	str r3, [r0], #4  /* [r0] = r3, r0 = r0 + 4 */
	cmp r0 ,r2
	bne 1b
	mov pc, lr

.align 4
mem_cfg:
.long 0x22000000
.long 0x0700
.long 0x0700
.long 0x0700
.long 0x0700
.long 0x0700
.long 0x0700
.long 0x18005
.long 0x18005
.long 0x8c04f5
.long 0xb2
.long 0x30
.long 0x30
