void delayms(unsigned int dly)
{
	volatile unsigned int n = dly * 100;
	while(n--);
}
