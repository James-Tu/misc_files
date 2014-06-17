#include<regs-gpio.h>
#include<regs-timer.h>
#include<io.h>
#include<stdio.h>
#include"irq.h"
#include"timer.h"
#include"serial.h"
#include"button.h"
#include"rtc.h"
#include"common.h"
#include"lcd.h"
#include"adc_ts.h"
#include"nor.h"
#include"nand.h"
#include"watchdog.h"
#include"i2c.h"

int main()
{	
	copy_vec();
	irq_init();
	button_init();
	uart0_init();
	timer_init();
//	timer4_init();

	lcd_init();
	lcd_clean(0xffff);
	adc_ts_init();
	ts_init();

//	tslib_calibrate();
//	lcd_clean(0xffff);

	nand_init();

	i2c_init();

	unsigned char buf[2] = {1, 0x5a};
	unsigned char addr = 1, data;

#define I2C_EEPROM_ADDR 0x50
	i2c_write(I2C_EEPROM_ADDR, buf, 2);	//往EEPROM地址1处写数据0x5a

	
	i2c_write(I2C_EEPROM_ADDR, &addr, 1);	// 写地址
	i2c_write(I2C_EEPROM_ADDR, &data, 1);	// 读数据

	printf("addr:0x%x data:0x%x\r\n", addr, data);
	
	while(1);


	return 0;
}

