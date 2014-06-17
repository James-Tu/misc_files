#ifndef __LCD_H
#define __LCD_H

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

extern void lcd_init();
extern void lcd_clean(unsigned short c);
extern void lcd_display_pic();

#endif
