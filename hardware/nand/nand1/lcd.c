#include<io.h>
#include<regs-clock.h>
#include<regs-lcd.h>
#include<regs-gpio.h>
#include"lcd.h"
#include"clock.h"

#define CLKVAL 8

#define VBPD 0
#define VFPD 4
#define VSPW 9

#define HBPD 25
#define HFPD 0
#define HSPW 4

static unsigned short lcd_buf[LCD_HEIGHT][LCD_WIDTH];
#define LCD_BUF_ADDR ((unsigned int)lcd_buf)

void lcd_init()
{
	//设置lcd pin脚功能
	writel(0xaaaaaaaa, GPCCON);
	writel(0xffffffff, GPCUP);
	writel(0xaaaaaaaa, GPDCON);
	writel(0xffffffff, GPDUP);

	clk_enable(CLKCON_LCDC, 1);
	
	//16BPP TFT LCD
	writel((CLKVAL << 8) | (3 << 5) | (12 << 1), LCDCON1);
	writel((VBPD << 24) | ((LCD_HEIGHT - 1) << 14) | (VFPD << 6) | VSPW, LCDCON2);
	writel((HBPD << 19) | ((LCD_WIDTH - 1) << 8) | HFPD, LCDCON3);
	writel(HSPW, LCDCON4);
	writel((1 << 11) | (1 << 6) | 1, LCDCON5);

	writel(((LCD_BUF_ADDR >> 22) << 21) | ((LCD_BUF_ADDR >> 1) & 0x1fffff), LCDSADDR1);
	writel(((LCD_BUF_ADDR >> 1) & 0x1fffff) + LCD_WIDTH * LCD_HEIGHT, LCDSADDR2);
	writel(LCD_WIDTH, LCDSADDR3);

	//不使用临时调色板
	writel(0, TPAL);
	//禁止lcd中断
	writel(3, LCDINTMSK);
	//disable LCC3600 和 LPC3600
	writel(0, TCONSEL);

	//打开lcd背光
	writel((readl(GPGCON) & ~(3 << 8)) | (1 << 8), GPGCON); //设置GPG4为输出功能
	writel(readl(GPGUP) | (1 << 4), GPGUP);
	writel(readl(GPGDAT) | (1 << 4), GPGDAT);  //设置GPG4输出1

	writel(readl(LCDCON1) | 0x1, LCDCON1);
}

void lcd_clean(unsigned short c)
{
	int i, j;

	for(j = 0; j < LCD_HEIGHT; j++)
		for(i = 0; i < LCD_WIDTH; i++)
			lcd_buf[j][i] = c;
}

extern const unsigned char gImage_pic_240x320[];
void lcd_display_pic()
{
	int i, j;
	unsigned short *data = (unsigned short *)gImage_pic_240x320;

	for(j = 0; j < LCD_HEIGHT; j++)
		for(i = 0; i < LCD_WIDTH; i++)
			lcd_buf[j][i] = *data++;
}

void lcd_draw_pixel(unsigned int x, unsigned int y, unsigned short c)
{
	if(x < LCD_WIDTH && y < LCD_HEIGHT)
		lcd_buf[y][x] = c;
}

