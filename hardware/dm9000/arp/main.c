#include<regs-gpio.h>
#include<regs-timer.h>
#include<io.h>
#include<stdio.h>
#include<string.h>
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
#include"cmif.h"
#include"i2s.h"
#include"dm9000.h"

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

	i2s_init();
	uda1341_init();

	dm9000_init();

	printf("Waiting for network connection established ... \r\n");
	printf("Press any key to send arp request.\r\n");
	getc();

	unsigned char recbuf[2048];
	unsigned char arpbuf[] = 
	{
		0xff,0xff,0xff,0xff,0xff,0xff, /* 以太网目的地址 */
		0x11,0x22,0x33,0x44,0x55,0x66, /* 以太网源地址 */
		0x08,0x06, /* 帧类型 */

		0x00,0x01, /* 硬件类型: 以太网地址 */
		0x08,0x00, /* 协议类型:  IP地址 */
		0x06, /* 硬件地址长度 */
		0x04, /* 协议地址长度 */
		0x00,0x01, /* 操作类型: A R P请求（值为1） */
		0x11,0x22,0x33,0x44,0x55,0x66, /* 发送端以太网地址 */
		192,168,1,24, /* 发送端IP地址 */
		0x00,0x00,0x00,0x00,0x00,0x00, /* 目的以太网地址 */
		192,168,1,14 /* 目的IP地址 */
	};

	dm9000_send(arpbuf, sizeof(arpbuf));

	while(!dm9000_rx(recbuf));

	if(recbuf[12] == 0x08 && recbuf[13] == 0x06 /* 帧类型: arp */
		&& recbuf[21] == 0x02 /*  操作类型: A R P应答（值为 2） */)
	{
		printf("host mac addr: %02x-%02x-%02x-%02x-%02x-%02x\r\n", recbuf[6],
		recbuf[7],recbuf[8],recbuf[9],recbuf[10],recbuf[11]);
	}


	while(1);

	return 0;
}

