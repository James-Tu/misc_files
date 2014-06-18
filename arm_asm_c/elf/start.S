.globl _start
_start:
    //¹Ø±Õwatchdog
    mov r0, #0x53000000
    mov r1, #0x0
    str r1, [r0]

    //ÉèÖÃÕ»Ö¸Õë¼Ä´æÆ÷sp
    mov sp, #4096
    bl main

 mainloop:
    b mainloop