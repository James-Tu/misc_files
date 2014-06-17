#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//#define DEBUG

int main(int argc ,char *argv[])
{
	int i;
	FILE *fp1, *fp2;
	unsigned char buf[3];
	unsigned char headinfo[54];
	unsigned short int data;
	int cnt = 0;

#ifdef DEBUG	
	for(i = 0; i < argc; i++)
	{
			printf("param%d:		%s\n", i, *(argv + i));
	}
#endif

	if(2 != argc)
	{
		printf("plase specify source bmp picture ... \n");
		exit(1);
	}

	if((fp1=fopen(argv[1],"r"))==NULL)
	{
		printf("open bmp file %x error ... \n", argv[1]);
		exit(1);
	}

	if((fp2=fopen("rgb565","w+"))==NULL)
	{
		printf("open dest file rgb565 error ... \n");
		exit(1);
	}
	
	fread(headinfo, 1, 54, fp1);
	
	while(fread(buf, 1, 3, fp1) == 3)
	{
		cnt++;
		data = (((buf[2] >> 3) & 0x1f) << 11) | (((buf[1] >> 2) & 0x3f) << 5) | ((buf[0] >> 3) | 0x1f);
		fwrite(&data, 1, 2, fp2);
	}
	
//	printf("cnt = %d		272*480=%d\n", cnt, 272*480);
	
	fclose(fp1);
	fclose(fp2);
	
	return 0;
}
