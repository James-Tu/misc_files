#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static inline void rgbyuv(unsigned char r, unsigned char g, unsigned char b, 
	unsigned char *py, unsigned char *pu, unsigned char *pv)
{
	int y, u, v;
/*
	y = 0.257 * r + 0.504 * g + 0.098 * b + 16
	u = -0.148 * r - 0.291 * g + 0.439 * b + 128
	v = 0.439 * r - 0.368 * g - 0.071 * b + 128
*/

	y = ((33 * r + 64 * g + 13 * b) >> 7) + 16;
	u = ((-19 * r - 37 * g + 56 * b) >> 7) + 128;
	v = ((56 * r - 47 * g - 9 * b) >> 7) + 128;

	if(y < 0)
		*py = 0;
	else if(y > 255)
		*py = 255;
	else
		*py = y;

	if(pu)
	{
		if(u < 0)
			*pu = 0;
		else if(u > 255)
			*pu = 255;
		else
			*pu = u;
	}
	
	if(pv)
	{
		if(v < 0)
			*pv = 0;
		else if(v > 255)
			*pv = 255;
		else
			*pv = v;		
	}
}

static int rgb888_2_yuv420planar(unsigned char* prgb, unsigned char* py, 
	unsigned char* pu, unsigned char* pv, int width, int height)
{
	int i, j;
	unsigned char r, g, b;
	
	for(i = 0; i < height; i = i + 2)
	{
		for(j = 0; j < width; j = j + 2)
		{
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, pu++, pv++);
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, NULL, NULL);
		}

		for(j = 0; j < width; j = j + 2)
		{
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, NULL, NULL);
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, NULL, NULL);
		}
	}
}

static int rgb888_2_yuv422planar(unsigned char* prgb, unsigned char* py, 
	unsigned char* pu, unsigned char* pv, int width, int height)
{
	int i, j;
	unsigned char r, g, b;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j = j + 2)
		{
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, pu++, pv++);
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, NULL, NULL);
		}
	}
}

static int rgb888_2_yuv444planar(unsigned char* prgb, unsigned char* py, 
	unsigned char* pu, unsigned char* pv, int width, int height)
{
	int i, j;
	unsigned char r, g, b;
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			b = *prgb++;	g = *prgb++;	r = *prgb++;
			rgbyuv(r, g, b, py++, pu++, pv++);
		}
	}
}


unsigned char bmp_head[] = {
0x42,0x4d,0x76,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
0x00,0x00,0xc8,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
0x00,0x00,0x40,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};


/*
	./rgb2yuv bmp888_filename yuv_format out_yuvfilename
*/
int main(int argc,char **argv)
{
	int width, height, i;
	FILE *fsrc, *fdest;
	unsigned char head[54];
	unsigned int datasize, fsize;
	unsigned char *src_buf;
	unsigned char *dest_buf;
	int (*pfn)(unsigned char*, unsigned char*, unsigned char*, unsigned char*, int, int);
	unsigned char *py, *pu, *pv;
	char outfilename[100];
/*
	printf("argc = %d\n", argc);
	for(i = 0; i < argc; i++)
	{
		printf("param%d:		%s\n", i, *(argv + i));
	}
*/
	if(4 != argc)
	{
		printf("\n*********************** rgb2yuv help *****************************\n");
		printf("./rgb2yuv   bmp888_filename   yuv_format   out_yuvfilename    \n\n");
		printf("bmp888_filename:             input bmp888 file name\n");
		printf("     yuv_format:             'yuv420' 'yuv422' 'yuv444'\n");
		printf("out_yuvfilename:             dest output yuv file name\n\n");
		return -1;
	}

	fsrc = fopen(*(argv + 1),"rb");
	if(!fsrc)
	{
		printf("open bmp888 file %s error ... \n", *(argv + 1));
		goto error;
	}	
	fread(head, 1, 54, fsrc);
	if(0x42 != head[0] || 0x4d != head[1])
	{
		printf("%s is not a bmp file ... \n", *(argv + 1));
		goto error;
	}
	
	memcpy(&width, head + 18, 4);
	memcpy(&height, head + 22, 4);
	if(height < 0)
		height = -1 * height;
	
	printf("width = %d	height = %d\n", width, height);
	
	if(!strcmp("yuv420", *(argv + 2)))
	{
		fsize = width * height * 3 / 2;
		
		dest_buf = (unsigned char*)malloc(fsize);	
		if(!dest_buf)
		{
			printf("malloc for dest_buf error ... \n");
			goto error;
		}
		
		py = dest_buf;
		pu = dest_buf + width * height;
		pv = pu + width * height / 4;
		
		pfn = rgb888_2_yuv420planar;
		sprintf(outfilename, "%s.yuv420", *(argv + 3));
	}
	else if(!strcmp("yuv422", *(argv + 2)))
	{
		fsize = width * height * 2;
		dest_buf = (unsigned char*)malloc(fsize);	
		if(!dest_buf)
		{
			printf("malloc for dest_buf error ... \n");
			goto error;
		}	
		py = dest_buf;
		pu = dest_buf + width * height;
		pv = pu + width * height / 2;
		
		pfn = rgb888_2_yuv422planar;
		sprintf(outfilename, "%s.yuv422", *(argv + 3));
	}
	else if(!strcmp("yuv444", *(argv + 2)))
	{
		fsize = width * height * 3;
		dest_buf = (unsigned char*)malloc(fsize);	
		if(!dest_buf)
		{
			printf("malloc for dest_buf error ... \n");
			goto error;
		}	
		py = dest_buf;
		pu = dest_buf + width * height;
		pv = pu + width * height;
		
		pfn = rgb888_2_yuv444planar;
		sprintf(outfilename, "%s.yuv444", *(argv + 3));
	}
	else
	{
		printf("invalid yuv format ... \n");
		goto error;
	}

	src_buf = (unsigned char*)malloc(width * height * 3);	
	if(!src_buf)
	{
		printf("malloc for src_buf error ... \n");
		goto error;
	}

	fread(src_buf, 1, width * height * 3, fsrc);
	fclose(fsrc);
	fsrc = NULL;

	pfn(src_buf, py, pu, pv, width, height);
	
	fdest = fopen(outfilename, "wb");
	if(!fdest)
	{
		printf("open yuv file %s error ... \n", outfilename);
		goto error;
	}
	fwrite(dest_buf, 1, fsize, fdest);
	fclose(fdest);	

error:
	if(fsrc)
		fclose(fsrc);
	if(src_buf)
		free(src_buf);
	if(dest_buf)
		free(dest_buf);
		
	return 0;
}
