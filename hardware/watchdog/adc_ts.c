#include<io.h>
#include<stdio.h>
#include<regs-adc.h>
#include<regs-clock.h>
#include<regs-irq.h>
#include"clock.h"
#include"common.h"
#include"lcd.h"

//触摸屏接口模式
#define TS_MODE_AUTO_XY		0xc
#define TS_MODE_WAIT_INT(up_down)	(0xd3 | ((up_down) << 8))
#define STYLUS_DOWN 0
#define STYLUS_UP 1

//A/D converter freq. = 50MHz/(49+1) = 1MHz
#define PRSCEN (1 << 14)
#define PRSCVL (49 << 6)

static volatile int A, B, C, D, E, F, DIV;
static volatile int ts_x, ts_y;
static volatile int ts_flag;
static volatile int ts_calibrated = 0;

void adc_ts_init()
{
	clk_enable(CLKCON_ADC, 1);
}

unsigned int read_adc(int ch)
{
	//设置ADC的时钟频率，选择输入通道，ch取值:0,1,2,3
	writel(PRSCEN | PRSCVL | (ch << 3), ADCCON);
	//启动AD转换
	writel(readl(ADCCON) | 0x1, ADCCON);
	//等待，直到AD转换真正开始
	while(readl(ADCCON) & 0x1);
	//等待，直到AD转换完成
	while(!(readl(ADCCON) & (1 << 15)));

	return readl(ADCDAT0) & 0x3ff;
}

void ts_init()
{
	//设置ADC的时钟频率
	writel(PRSCEN | PRSCVL, ADCCON);
	//采样延时: (1 / 3.68M) * 50000 = 13.58ms
	writel(50000, ADCDLY);
	//进入等待中断模式，等待触摸屏被按下
	writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
}

void sub_int_tc()
{
	printf("sub_int_tc\r\n");

	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_tc: Stylus up state\r\n");
		//进入等待中断模式，等待触摸屏被按下
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
		ts_flag = 1;
	}
	else
	{
		printf("sub_int_tc: Stylus down state\r\n");
		//进入自动连续的X，Y坐标转换模式
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//启动AD转换
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//清除中断:IRQ_SUB_TC
	writel(1 << IRQ_SUB_TC_OFT, SUBSRCPND);
}

void lcd_draw_point(int x, int y, unsigned short c)
{
	int i, j;
	for(j = y -2; j <= y + 2; j++)
		for(i = x - 2; i <= x + 2; i++)
			lcd_draw_pixel(i, j, c);
}

void sub_int_adc()
{
	int x, y;
	int xl, yl;   //用于存储显示屏的坐标
	printf("sub_int_adc\r\n");

	x = readl(ADCDAT0) & 0x3ff;
	y = readl(ADCDAT1) & 0x3ff;
	printf("sub_int_adc: [%d,%d]\r\n", x, y);

	//进入等待中断模式，等待触摸屏被释放
	writel(TS_MODE_WAIT_INT(STYLUS_UP), ADCTSC);

	ts_x = x;
	ts_y = y;

	if(ts_calibrated)
	{
		xl = (x * A + y * B + C) / DIV;
		yl = (x * D + y * E + F) / DIV;

		if(xl < 0)
			xl = 0;
		else if(xl >= LCD_WIDTH)
			xl = LCD_WIDTH -1;

		if(yl < 0)	
			yl = 0;
		else if(yl >= LCD_HEIGHT)
			yl = LCD_HEIGHT - 1;

		printf("lcd [%d,%d]\r\n", xl, yl);

		lcd_draw_point(xl, yl, 0xf800);
	}

	delayms(20);
	if(readl(ADCDAT0) & (1 << 15))
	{
		printf("sub_int_adc: Stylus up state\r\n");
		//进入等待中断模式，等待触摸屏被按下
		writel(TS_MODE_WAIT_INT(STYLUS_DOWN), ADCTSC);
	}
	else
	{
		printf("sub_int_adc: Stylus down state\r\n");
		//进入自动连续的X，Y坐标转换模式
		writel(TS_MODE_AUTO_XY, ADCTSC);
		//启动AD转换
		writel(readl(ADCCON) | 0x1, ADCCON);
	}

	//清除中断:IRQ_SUB_ADC
	writel(1 << IRQ_SUB_ADC_S_OFT, SUBSRCPND);
}

void int_adc_handle()
{
	printf("int_adc_handle\r\n");
	
	if(readl(SUBSRCPND) & (1 << IRQ_SUB_TC_OFT))
		sub_int_tc();

	if(readl(SUBSRCPND) & (1 << IRQ_SUB_ADC_S_OFT))
		sub_int_adc();
}

// TSLIB

typedef struct {
	int x[5], xfb[5];
	int y[5], yfb[5];
	int a[7];
} calibration;

int perform_calibration(calibration *cal) {
	int j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float scaling = 65536.0;

// Get sums for matrix
	n = x = y = x2 = y2 = xy = 0;
	for(j=0;j<5;j++) {
		n += 1.0;
		x += (float)cal->x[j];
		y += (float)cal->y[j];
		x2 += (float)(cal->x[j]*cal->x[j]);
		y2 += (float)(cal->y[j]*cal->y[j]);
		xy += (float)(cal->x[j]*cal->y[j]);
	}

// Get determinant of matrix -- check if determinant is too small
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if(det < 0.1 && det > -0.1) {
//		printf("ts_calibrate: determinant is too small -- %f\n",det);
		return 0;
	}

// Get elements of inverse matrix
	a = (x2*y2 - xy*xy)/det;
	b = (xy*y - x*y2)/det;
	c = (x*xy - y*x2)/det;
	e = (n*y2 - y*y)/det;
	f = (x*y - n*xy)/det;
	i = (n*x2 - x*x)/det;

// Get sums for x calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) {
		z += (float)cal->xfb[j];
		zx += (float)(cal->xfb[j]*cal->x[j]);
		zy += (float)(cal->xfb[j]*cal->y[j]);
	}

// Now multiply out to get the calibration for framebuffer x coord
	cal->a[0] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[1] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[2] = (int)((c*z + f*zx + i*zy)*(scaling));
/*
	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));
*/
// Get sums for y calibration
	z = zx = zy = 0;
	for(j=0;j<5;j++) {
		z += (float)cal->yfb[j];
		zx += (float)(cal->yfb[j]*cal->x[j]);
		zy += (float)(cal->yfb[j]*cal->y[j]);
	}

// Now multiply out to get the calibration for framebuffer y coord
	cal->a[3] = (int)((a*z + b*zx + c*zy)*(scaling));
	cal->a[4] = (int)((b*z + e*zx + f*zy)*(scaling));
	cal->a[5] = (int)((c*z + f*zx + i*zy)*(scaling));
/*
	printf("%f %f %f\n",(a*z + b*zx + c*zy),
				(b*z + e*zx + f*zy),
				(c*z + f*zx + i*zy));
*/
// If we got here, we're OK, so assign scaling to a[6] and return
	cal->a[6] = (int)scaling;
	return 1;
/*	
// This code was here originally to just insert default values
	for(j=0;j<7;j++) {
		c->a[j]=0;
	}
	c->a[1] = c->a[5] = c->a[6] = 1;
	return 1;
*/

}

void lcd_draw_cross(int x, int y, unsigned short c)
{
	int i;
	
	lcd_clean(0xffff);
	for(i = x - 10; i <= x + 10; i++)
		lcd_draw_pixel(i, y, c);

	for(i = y - 10; i <= y + 10; i++)
		lcd_draw_pixel(x, i, c);
}

static void get_sample (calibration *cal,
			int index, int x, int y, char *name)
{
 	lcd_draw_cross(x, y, 0xf800);
	ts_flag = 0;
	while(ts_flag == 0);

	cal->x [index] = ts_x;
	cal->y [index] = ts_y;

	cal->xfb [index] = x;
	cal->yfb [index] = y;

	printf("%s : X = %4d Y = %4d\n", name, cal->x [index], cal->y [index]);
	delayms(500);
}

int tslib_calibrate()
{
	calibration cal;
	unsigned int i;
	int xres = LCD_WIDTH;
	int yres = LCD_HEIGHT;

	ts_calibrated = 0;
	
	get_sample (&cal, 0, 50,        50,        "Top left");
	get_sample (&cal, 1, xres - 50, 50,        "Top right");
	get_sample (&cal, 2, xres - 50, yres - 50, "Bot right");
	get_sample (&cal, 3, 50,        yres - 50, "Bot left");
	get_sample (&cal, 4, xres / 2,  yres / 2,  "Center");

	if (perform_calibration (&cal)) {
		printf ("%d %d %d %d %d %d %d\r\n",
			 cal.a[1], cal.a[2], cal.a[0],
			 cal.a[4], cal.a[5], cal.a[3], cal.a[6]);
                i = 0;
        A = cal.a[1]; B = cal.a[2]; C = cal.a[0];
		D = cal.a[4]; E = cal.a[5]; F= cal.a[3]; DIV = cal.a[6];
	} else {
		printf("Calibration failed.\n");
		i = -1;
	}

	ts_calibrated = 1;

	return i;
}

