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

	//�����ڲ�PHY
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

	//����mac��ַ
	for(i = 0; i < 6; i++)
		dm9000_reg_write(DM9000_PAR + i, mac_addr[i]);

	dm9000_reg_write(DM9000_IMR, 0x80); /* ���������жϣ����ò�ѯ��ʽ */
	dm9000_reg_write(DM9000_RCR, 0x31);	/* ʹ�ܽ������ݰ����� */
}

int eth_send(unsigned char *datas, int len)
{
	int i;
	unsigned short *ds = (unsigned short *)datas;

	dm9000_reg_write(DM9000_ISR, 1 << 1);

	//�����ݰ�д��TX FIFO SRAM
	writeb(DM9000_MWCMD, DM9000_ADDR_PORT);
	for(i = 0; i < len; i += 2)
		writew(*ds++, DM9000_DATA_PORT);

	//�趨�������ݰ��ĳ���
	dm9000_reg_write(DM9000_TXPLL, len & 0xff);
	dm9000_reg_write(DM9000_TXPLH, (len >> 8) & 0xff);

	//�������͹���
	dm9000_reg_write(DM9000_TCR, 0x1);

	//�ȴ����ݰ��������
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

