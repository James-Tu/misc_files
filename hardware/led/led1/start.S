.globl _start
_start:
    //¹Ø±Õwatchdog
    ldr r0, =0x53000000
    ldr r1, =0x0
    str r1, [r0]

    //ÉèÖÃÕ»Ö¸Õë¼Ä´æÆ÷sp
    ldr sp, =4096
    bl main

 mainloop:
    b mainloop