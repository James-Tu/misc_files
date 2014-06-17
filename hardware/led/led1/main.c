#define GPBCON (*((volatile unsigned int*)0x56000010))
#define GPBDAT (*((volatile unsigned int*)0x56000014))

void main()
{
    volatile unsigned int n;
    
    GPBCON = 0x15400;

    while(1)
    {
#if 0    
        GPBDAT = 0x0; //点亮4个led灯
        n = 12 * 4096;
        while(n--);

        GPBDAT = 0xffff; //熄灭4个led灯
        n = 12 * 4096;
        while(n--);
#endif
        GPBDAT = ~(1 << 5); //点亮第1个led灯
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 6); //点亮第2个led灯
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 7); //点亮第3个led灯
        n = 12 * 4096;
        while(n--);

        GPBDAT = ~(1 << 8); //点亮第4个led灯
        n = 12 * 4096;
        while(n--);
    }
}
