int g_test[10];

int main()
{
	int i;
	uart0_init();

	printf("uart test\r\n");

	for(i = 0; i < 10; i++)
		printf("g_test[%d] = %d\r\n", i, g_test[i]);

	while(1)
	{
		getc();
	}

	return 0;
}
