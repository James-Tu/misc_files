#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int fd;
	unsigned char value;
	
	if(3 != argc)
	{
		printf("\nusage : ./leds_test dev_node  value\n\n");
		printf("dev_node: leds device node\n");
		printf("value   : <=0xf: set leds on/off, each bit of value(lower 4 bit) indicate set leds on or off\n");
		printf("          > 0xf: read leds status\n");
		return -1;
	}

	value = strtoul(*(argv + 2), NULL, 16);
	
	
	if ((fd = open(*(argv + 1), O_RDWR)) < 0)
	{
		printf("open dev: %s error\n", *(argv + 1));
		return -1;
	}

	if(value > 0xf )		//read leds status
	{
		read(fd, &value, 1);
		printf("leds status = 0x%x\n", value);
	}
	else		// set leds on or off
	{
		printf("value = 0x%x\n", value);
		write(fd, &value, 1);
	}
	
	close(fd);
	
	return 0;
}
