#include<sys/ioctl.h>
#include<sys/mman.h>
#include<string.h>
#include<linux/fb.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// ./test_fb /dev/fb0 rgb565
int main(int argc, char **argv)
{
	int fbd;
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	FILE *file_fd = NULL;
	unsigned char *pfb = NULL;
	int ret, file_len;
	
	if(3 != argc)
	{
		printf("./test_fb /dev/fb0 rgb565");
		return -1;
	}
	
	if((fbd = open(*(argv + 1), O_RDWR)) < 0)
	{
		printf("open %s error\n", *(argv + 1));
		return -1;
	}
		
	ioctl(fbd, FBIOGET_VSCREENINFO, &var);
	ioctl(fbd, FBIOGET_FSCREENINFO, &fix);
	
	printf("xres = %d,	yres = %d\n", var.xres, var.yres);
	printf("bits_per_pixel = %d\n", var.bits_per_pixel);
	
/*
	size = var.xres * var.yres * var.bits_per_pixel / 8;
	printf("size = %d, 	smem_len = %d\n", size, fix.smem_len);
*/

	if((unsigned char*)-1 == (pfb = (unsigned char*)mmap(NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbd, 0)))
	{
		printf("mmap error.\n");
		return -1;
	}
	
	
	if (NULL == (file_fd = fopen(*(argv + 2), "rb")))
	{
		printf("fopen failed file_fd = %p\n", file_fd);
		return -1;
	}
	fseek(file_fd, 0, SEEK_END); //定位到文件末  
	file_len = ftell(file_fd); //文件长度 
	printf("file_len = %d\n", file_len);
	
	fseek(file_fd, 0, SEEK_SET);
	ret = fread(pfb, 1, file_len, file_fd);
	printf("fread size =%d\n", ret);
	
	fclose(file_fd);

	close(fbd);	
	return -1;
}