#include<io.h>
#include<stdio.h>
#include<net.h>
#include"common.h"
#include"dm9000.h"

void dm9000_reg_write(unsigned char reg, unsigned char data)
{
	writeb(reg, DM9000_ADDR_PORT);
	writeb(data, DM9000_DATA_PORT);
}

unsigned char dm9000_reg_read(unsigned char reg)
{
	writeb(reg, DM9000_ADDR_PORT);
	return readb(DM9000_DATA_PORT);
}

void eth_init()
{
	unsigned short vid;
	int i;
	
	vid = (dm9000_reg_read(DM9000_VIDH) << 8) | dm9000_reg_read(DM9000_VIDL);
	if(0x0a46 == vid)
		printf("dm9000 device found ... \r\n");
	else
	{
		printf("dm9000 device not found ... \r\n");
		return;
	}

	//激活内部PHY
	dm9000_reg_write(DM9000_GPCR, 0x01);
	dm9000_reg_write(DM9000_GPR, 0x0);

	//reset dm9000
	dm9000_reg_write(DM9000_NCR, 0x3);
	delayms(1);
	dm9000_reg_write(DM9000_NCR, 0x0);
	
	dm9000_reg_write(DM9000_NCR, 0x3);
	delayms(1);
	dm9000_reg_write(DM9000_NCR, 0x0);

	//clear TX status
	dm9000_reg_write(DM9000_NSR, 0x2c);

	//设置mac地址
	for(i = 0; i < 6; i++)
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);

	dm9000_reg_write(DM9000_IMR, 0x80); /* 屏蔽所有中断，采用查询方式 */
	dm9000_reg_write(DM9000_RCR, 0x31);	/* 使能接收数据包功能 */
}

int eth_send(unsigned char *datas, int len)
{
	int i;
	unsigned short *ds = (unsigned short *)datas;

	dm9000_reg_write(DM9000_ISR, 1 << 1);

	//把数据包写到TX FIFO SRAM
	writeb(DM9000_MWCMD, DM9000_ADDR_PORT);
	for(i = 0; i < len; i += 2)
		writew(*ds++, DM9000_DATA_PORT);

	//设定发送数据包的长度
	dm9000_reg_write(DM9000_TXPLL, len & 0xff);
	dm9000_reg_write(DM9000_TXPLH, (len >> 8) & 0xff);

	//启动发送功能
	dm9000_reg_write(DM9000_TCR, 0x1);

	//等待数据包发送完成
	while(!(dm9000_reg_read(DM9000_ISR) & (1 << 1)));
	while(!(dm9000_reg_read(DM9000_NSR) & 0xc));

	return len;
}

int eth_rx(unsigned char *datas)
{
	unsigned char ready;
	int len, status;
	int i;
	unsigned short *ds = (unsigned short *)datas;
	
	ready = dm9000_reg_read(DM9000_MRCMDX);
	if(ready > 0x01)
	{
		eth_init();
		return 0;
	}

	if(ready != 0x01)
		return 0;

	writeb(DM9000_MRCMD, DM9000_ADDR_PORT);
	status = readw(DM9000_DATA_PORT);
	len = readw(DM9000_DATA_PORT);

	for(i = 0; i < len; i += 2)
		*ds++ = readw(DM9000_DATA_PORT);

	net_receive(datas, len);
	return len;
}

