#define GPBCON (*((volatile unsigned int*)0x56000010))
#define GPBDAT (*((volatile unsigned int*)0x56000014))

void main()
{
    volatile unsigned int n;
    
    GPBCON = 0x15400;

    while(1)
    {
#if 0    
        GPBDAT = 0x0; //����4��led��
        n = 12 * 4096;
        while(n--);

        GPBDAT = 0xffff; //Ϩ��4��led��
        n = 12 * 4096;
        while(n--);
#endif
        GPBDAT = ~(1 << 5); //������1��led��
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 6); //������2��led��
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 7); //������3��led��
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 8); //������4��led��
        n = 12 * 4096;
        while(n--);
    }
}
