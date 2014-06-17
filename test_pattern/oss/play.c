#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

//./play /dev/dsp sbq.pcm
int main(int argc,char *argv[])
{
	int dsp_fd, file_len;
	unsigned char *buf = NULL;
	FILE *file_fd;
	int ret;
	
	/* 打开并计算文件长度* */
	if (NULL == (file_fd = fopen(*(argv + 2), "rb")))
	{
		printf("fopen failed file_fd = %p\n", file_fd);
		return -1;
	}
	fseek(file_fd, 0, SEEK_END); //定位到文件末  
	file_len = ftell(file_fd); //文件长度 
	printf("file_len = %d\n", file_len);
	
	buf = malloc(file_len);
	if(NULL == buf)
	{
		printf("malloc error\n");
		return -1;
	}
	fseek(file_fd, 0, SEEK_SET);
	ret = fread(buf, 1, file_len, file_fd);
	printf("fread size =%d\n", ret);
	
	printf("buf[0] = %x\n", buf[0]);
	printf("buf[1] = %x\n", buf[1]);

	dsp_fd = open(*(argv + 1), O_RDWR);
	if(dsp_fd < 0)
	{
		printf("open dev: %s error\n", *(argv + 1));
		return -1;
	}
	
	ret = write(dsp_fd, buf, file_len);

	printf("write count: %d\n", ret);
	
	fclose(file_fd);
	close(dsp_fd);
	return 0;
}