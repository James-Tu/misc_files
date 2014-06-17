#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum
{
	FMT_RGB565 = 0x01,
	FMT_ARGB1555 = 0x02,
	
	FMT_RGB888 = 0x04,
	FMT_BGR888 = 0x08,
	
	FMT_ARGB8888 = 0x10,
	FMT_ABGR8888 = 0x20,
	
	FMT_RGBA8888 = 0x40,
	FMT_BGRA8888 = 0x80,
	
	FMT_RGB_MASK = 0xff
};

static inline int yuvrgb(unsigned char y, unsigned char u, unsigned char v, unsigned char* prgb, unsigned int fmt)
{
	int r, g, b;
	int temp;

	if(!prgb || !(FMT_RGB_MASK & fmt))
	{
		printf("yuvrgb: null point or invalid color fmt ... \n");
		return -1;
	}
/*
	r= 1.164 * (y - 16) + 1.159 * (v - 128);
	g= 1.164 * (y - 16) - 0.38 * (u - 128) - 0.813 * (v - 128);
	b= 1.164 * (y - 16) + 2.018 * (u - 128);
*/

	r= (149 * (y - 16) + 148 * (v - 128)) >> 7;	
	g= (149 * (y - 16) - 49 * (u - 128) - 104 * (v - 128)) >> 7;
	b= (149 * (y - 16) + 255 * (u - 128)) >> 7;

	if(r > 255)
		r = 255;
	else if(r < 0)
		r = 0;

	if(g > 255)
		g = 255;
	else if(g < 0)
		g = 0;
	
	if(b > 255)
		b = 255;
	else if(b < 0)
		b = 0;

	if(fmt & (FMT_BGR888 | FMT_ABGR8888 |FMT_BGRA8888))
	{
		temp = r;
		r = b;
		b = temp;
	}

	if(fmt & FMT_RGB565)
	{
		*((unsigned short*)prgb) = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
	}
	else if(fmt & FMT_ARGB1555)
	{
		*((unsigned short*)prgb) = (1 << 15) | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
	}
	else if(fmt & (FMT_RGB888 | FMT_BGR888))
	{
		*prgb++ = r;	*prgb++ = g;	*prgb++ = b;
	}
	else if(fmt & (FMT_ARGB8888 | FMT_ABGR8888))
	{
		*((unsigned int*)prgb) = (0xff << 24) | (r << 16) | (g << 8) | b;
	}
	else if(fmt & (FMT_RGBA8888 | FMT_BGRA8888))
	{
		*((unsigned int*)prgb) = (r << 24) | (g << 16) | (b << 8) | (0xff);
	}
	
	return 0;
}

static int yuv420planar_2_rgb(unsigned char* py, unsigned char* pu, unsigned char* pv, unsigned char* prgb, int width, int height, unsigned int fmt)
{
	int i, j;
	int n;
	
	if(!prgb || !(FMT_RGB_MASK & fmt))
	{
		printf("yuv420planar_2_rgb: null point or invalid color fmt ... \n");
		return -1;
	}
	
	if((FMT_RGB565 | FMT_ARGB1555) & fmt)
		n = 2;
	else if((FMT_RGB888 | FMT_BGR888) & fmt)
		n = 3;
	else
		n = 4;
	
	for(i = 0; i < height; i = i + 2)
	{
		for(j = 0; j < width; j++)
		{
			yuvrgb(*py++, *(pu + j / 2), *(pv + j / 2), prgb, fmt);
			prgb  = prgb + n;
		}

		for(j = 0; j < width; j++)
		{
			yuvrgb(*py++, *(pu + j / 2), *(pv + j / 2), prgb, fmt);
			prgb  = prgb + n;
		}
		
		pu = pu + width / 2;
		pv = pv + width / 2;
	}
	
	return 0;
}

static int yuv422interleaved_2_rgb(unsigned char* pyuv /*YUYV or YVYU*/, 
	unsigned char* prgb, int width, int height, unsigned int fmt)
{
	int i, j;
	int n;
	
	if(!prgb || !(FMT_RGB_MASK & fmt))
	{
		printf("yuv422interleaved_2_rgb: null point or invalid color fmt ... \n");
		return -1;
	}
	
	if((FMT_RGB565 | FMT_ARGB1555) & fmt)
		n = 2;
	else if((FMT_RGB888 | FMT_BGR888) & fmt)
		n = 3;
	else
		n = 4;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j = j + 2)
		{
			yuvrgb(*pyuv, *(pyuv + 1), *(pyuv + 3), prgb, fmt);
			prgb  = prgb + n;

			yuvrgb(*(pyuv + 2), *(pyuv + 1), *(pyuv + 3), prgb, fmt);
			prgb  = prgb + n;

			pyuv += 4;
		}
	}
	
	return 0;
}

static int yuv422planar_2_rgb(unsigned char* py, unsigned char* pu, unsigned char* pv, unsigned char* prgb, int width, int height, unsigned int fmt)
{
	int i, j;
	int n;
	
	if(!prgb || !(FMT_RGB_MASK & fmt))
	{
		printf("yuv422planar_2_rgb: null point or invalid color fmt ... \n");
		return -1;
	}
	
	if((FMT_RGB565 | FMT_ARGB1555) & fmt)
		n = 2;
	else if((FMT_RGB888 | FMT_BGR888) & fmt)
		n = 3;
	else
		n = 4;


	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			yuvrgb(*py++, *(pu + j / 2), *(pv + j / 2), prgb, fmt);
			prgb  = prgb + n;
		}

		pu = pu + width / 2;
		pv = pv + width / 2;
	}
	
	return 0;
}

static int yuv444planar_2_rgb(unsigned char* py, unsigned char* pu, unsigned char* pv,
	unsigned char* prgb, int width, int height, unsigned int fmt)
{
	int i, j;
	int n;
	
	if(!prgb || !(FMT_RGB_MASK & fmt))
	{
		printf("yuv444planar_2_rgb: null point or invalid color fmt ... \n");
		return -1;
	}
	
	if((FMT_RGB565 | FMT_ARGB1555) & fmt)
		n = 2;
	else if((FMT_RGB888 | FMT_BGR888) & fmt)
		n = 3;
	else
		n = 4;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			yuvrgb(*py++, *pu++, *pv++, prgb, fmt);
			prgb  = prgb + n;
		}
	}
	
	return 0;
}

static const unsigned char g_bmp16_head[] = 
{
	0x42,0x4d,0x76,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
	0x00,0x00,0xc8,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x01,0x00,0x10,0x00,0x00,0x00,
	0x00,0x00,0x40,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char g_bmp24_head[] = 
{
	0x42,0x4d,0x76,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
	0x00,0x00,0xc8,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
	0x00,0x00,0x40,0x19,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00
};

/*
	./yuv2rgb yuv_filename yuv_format width height outfilename
*/
int main(int argc,char **argv)
{
	int width, height, i;
	FILE *fsrc, *fdest;
	unsigned char head[54];
	unsigned int datasize, fsize;
	unsigned char *src_buf = NULL;
	unsigned char *dest_buf = NULL;
	int (*pfn)(unsigned char*, unsigned char*, unsigned char*, unsigned char*, int, int, unsigned int);
	unsigned char *py, *pu, *pv;
	int pic_h, pic_w;
	char outfilename[100];

/*
	printf("argc = %d\n", argc);
	for(i = 0; i < argc; i++)
	{
		printf("param%d:		%s\n", i, *(argv + i));
	}
*/

	if(6 != argc)
	{
		printf("\n*************************** yuv2rgb help ***************************\n");
		printf("./yuv2rgb   yuv_filename   yuv_format   width   height   outfilename\n\n");
		printf("yuv_filename:             input yuv file name\n");
		printf("  yuv_format:             'yuv420' 'yuv422' 'yuv444'\n");
		printf("       width:             dest picture width\n");
		printf("      height:             dest picture height\n");
		printf(" outfilename:             dest output bmp888 file name\n");
		return -1;
	}
	
	width = strtoul(*(argv + 3), NULL, 10);
	height = strtoul(*(argv + 4), NULL, 10);
//	printf("width = %d	height = %d\n", width, height);
	
	if(!width || !height || (width > 10000) || (height > 10000))
	{
		printf("invalid width or height ... \n");
	}
	
	if(!strcmp("yuv420", *(argv + 2)))
	{
		fsize = width * height * 3 / 2;
		
		src_buf = (unsigned char*)malloc(fsize);	
		if(!src_buf)
		{
			printf("malloc for src_buf error ... \n");
			goto error;
		}
		
		py = src_buf;
		pu = src_buf + width * height;
		pv = pu + width * height / 4;
		
		pfn = yuv420planar_2_rgb;
	}
	else if(!strcmp("yuv422", *(argv + 2)))
	{
		fsize = width * height * 2;
		src_buf = (unsigned char*)malloc(fsize);	
		if(!src_buf)
		{
			printf("malloc for src_buf error ... \n");
			goto error;
		}	
		py = src_buf;
		pu = src_buf + width * height;
		pv = pu + width * height / 2;
		
		pfn = yuv422planar_2_rgb;
	}
	else if(!strcmp("yuv444", *(argv + 2)))
	{
		fsize = width * height * 3;
		src_buf = (unsigned char*)malloc(fsize);	
		if(!src_buf)
		{
			printf("malloc for src_buf error ... \n");
			goto error;
		}	
		py = src_buf;
		pu = src_buf + width * height;
		pv = pu + width * height;
		
		pfn = yuv444planar_2_rgb;
	}
	else
	{
		printf("invalid yuv format ... \n");
		goto error;
	}

	fsrc = fopen(*(argv + 1),"rb");
	if(!fsrc)
	{
		printf("open yuv file %s error ... \n", *(argv + 1));
		goto error;
	}
	fseek(fsrc, 0, SEEK_END);
/*	
	if(fsize != ftell(fsrc))
	{
		printf("%s file length is not format width, height and yuv format ... \n");
		fclose(fsrc);
		goto error;
	}
*/

	if(fsize > ftell(fsrc))
	{
		printf("%s file length is not format width, height and yuv format ... \n");
		fclose(fsrc);
		goto error;
	}
	
	fseek(fsrc, 0, SEEK_SET);
	fread(src_buf, 1, fsize, fsrc);
	fclose(fsrc);
		
	//rgb 888
	memcpy(head, g_bmp24_head, 54);

	dest_buf = (unsigned char*)malloc(width * height * 3);
	if(!dest_buf)
	{
		printf("malloc for dest_buf error ... \n");
		goto error;
	}
	
	pfn(py, pu, pv, dest_buf, width, height, FMT_BGR888);

	datasize = (unsigned int)(width * height * 3);
	fsize  = datasize+54;
	pic_w = width;
	pic_h = -1 * height;
//	pic_h = height;
	head[2] = (unsigned char)fsize;
	head[3] = (unsigned char)(fsize>>8);
	head[4] = (unsigned char)(fsize>>16);
	head[5] = (unsigned char)(fsize>>24);
	head[18] = (unsigned char)pic_w;
	head[19] = (unsigned char)(pic_w>>8);
	head[20] = (unsigned char)(pic_w>>16);
	head[21] = (unsigned char)(pic_w>>24);
	head[22] = (unsigned char)pic_h;
	head[23] = (unsigned char)(pic_h>>8);
	head[24] = (unsigned char)(pic_h>>16);
	head[25] = (unsigned char)(pic_h>>24);
	head[34] = (unsigned char)datasize;
	head[35] = (unsigned char)(datasize>>8);
	head[36] = (unsigned char)(datasize>>16);
	head[37] = (unsigned char)(datasize>>24);
	
	sprintf(outfilename, "%s.bmp", *(argv + 5));
	fdest = fopen(outfilename, "wb");
	if(!fdest)
	{
		printf("open bmp888 file %s error ... \n", outfilename);
		goto error;
	}
	fwrite(head, 1, 54, fdest);
	fwrite(dest_buf, 1, datasize, fdest);
	fclose(fdest);	

error:
	if(src_buf)
		free(src_buf);
	if(dest_buf)
		free(dest_buf);
		
	return 0;
}

