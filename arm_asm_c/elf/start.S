.globl _start
_start:
    //�ر�watchdog
    mov r0, #0x53000000
    mov r1, #0x0
    str r1, [r0]

    //����ջָ��Ĵ���sp
    mov sp, #4096
    bl main

 mainloop:
    b mainloop