#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE (512 * 1024)		// 0.5M byte

//./record /dev/dsp sbq.pcm cnt
int main(int argc,char *argv[])
{
	int dsp_fd, file_fd;
	unsigned char buf[BUFFER_SIZE];
	int ret;
	int cnt;
	int i;
	
	if((file_fd = open(*(argv + 2), O_RDWR | O_CREAT)) < 0)
	{
		printf("open dev: %s error\n", *(argv + 2));
		return -1;
	}
	
	dsp_fd = open(*(argv + 1), O_RDWR);
	if(dsp_fd < 0)
	{
		printf("open dev: %s error\n", *(argv + 1));
		return -1;
	}
	
	sscanf(*(argv + 3), "%d", &cnt);
	
	printf("cnt = %d\n", cnt);
	
	while(cnt--)
	{
		memset(buf, 0, BUFFER_SIZE);
		
		ret = read(dsp_fd, buf, BUFFER_SIZE);
		printf("read count: %d\n", ret);
		
		write(file_fd, buf, BUFFER_SIZE);
#if 0		
		for(i = 0; i < 32; i++)
			printf("buf[%d] = 0x%x\n", i, buf[i]);
#endif
	}
	
	close(file_fd);
	close(dsp_fd);
	return 0;
}