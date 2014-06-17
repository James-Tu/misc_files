#include<io.h>
#include<stdio.h>
#include<regs-gpio.h>
#include<regs-clock.h>
#include<regs-cmif.h>
#include"clock.h"
#include"common.h"
#include"i2c.h"
#include"ov9650.h"

//计算主突发长度和剩余突发长度，用于CIPRCTRL(DMA控制相关)寄存器
void CalculateBurstSize(unsigned int hSize,unsigned int *mainBurstSize,
	unsigned int *remainedBurstSize)
{
	unsigned int tmp;
	
	tmp=(hSize/4)%16;
	switch(tmp)
	{
		case 0:
			*mainBurstSize=16;
			*remainedBurstSize=16;
			break;
		case 4:
			*mainBurstSize=16;
			*remainedBurstSize=4;
			break;
		case 8:
			*mainBurstSize=16;
			*remainedBurstSize=8;
			break;
		default:
			tmp=(hSize/4)%8;
			switch(tmp)
			{
			case 0:
				*mainBurstSize=8;
				*remainedBurstSize=8;
				break;
			case 4:
				*mainBurstSize=8;
				*remainedBurstSize=4;
			default:
				*mainBurstSize=4;
				tmp=(hSize/4)%4;
				*remainedBurstSize= (tmp) ? tmp: 4;
				break;
			}
			break;
	}
}

//计算预缩放比率及移位量
void CalculatePrescalerRatioShift(unsigned int SrcSize, 
	unsigned int DstSize, unsigned int *ratio,unsigned int *shift)
{
	if(SrcSize>=64*DstSize)
	{
		printf("ERROR:out of the prescaler range: SrcSize/DstSize = %d\r\n",SrcSize/DstSize);
		while(1);
	}
	else if(SrcSize>=32*DstSize)
	{
		*ratio=32;
		*shift=5;
	}
	else if(SrcSize>=16*DstSize)
	{
		*ratio=16;
		*shift=4;
	}
	else if(SrcSize>=8*DstSize)
	{
		*ratio=8;
		*shift=3;
	}
	else if(SrcSize>=4*DstSize)
	{
		*ratio=4;
		*shift=2;
	}
	else if(SrcSize>=2*DstSize)
	{
		*ratio=2;
		*shift=1;
	}
	else
	{
		*ratio=1;
		*shift=0;
	}
}

void ov9650_init()
{
	int i;
	unsigned char buf[2];
	unsigned char midh, midl;
	unsigned char ov9650_addr = 0x30;

	//Manufacturer ID Byte – High  reg[0x1c] = 0x7F
	buf[0] = 0x1c;
	i2c_write(ov9650_addr, buf, 1);
	i2c_read(ov9650_addr, &midh, 1);

	//Manufacturer ID Byte – Low  reg[0x1d] = 0xA2
	buf[0] = 0x1d;
	i2c_write(ov9650_addr, buf, 1);
	i2c_read(ov9650_addr, &midl, 1);

	printf("midh = 0x%x  midl = 0x%x\r\n", midh, midl);

	for(i = 0; i < OV9650_INIT_REGS; i++)
	{
		buf[0] = OV9650_init_reg[i].regaddr;
		buf[1] = OV9650_init_reg[i].value;
		i2c_write(ov9650_addr, buf, 2);
	}
}

void cmif_init()
{
	//初始化GPJ pin脚功能为camera功能
	writel(0x2aaaaaa, GPJCON);
	writel(0x0, GPJUP);

	//Camera clock = UPLL / [(CAMCLK_DIV +1)x2] = 24MHZ
	//UPLL = 96MHZ,  CAMCLK_DIV = 1
	writel((readl(CAMDIVN) & ~0x1f) | ((1 << 4) | 1), CAMDIVN);

	clk_enable(CLKCON_CAMERA, 1);

	writel((readl(GPGCON) & ~(3 << 24)) | (1 << 24), GPGCON); /* 设置GPG12为输出模式 */
	writel(readl(GPGDAT) | (1 << 12), GPGDAT);	/* GPG12输出1， ov9650 Power down mode */
	delayms(10);
	writel(readl(GPGDAT) & ~(1 << 12), GPGDAT);	/* GPG12输出0， ov9650 Normal mode */
	delayms(10);

	//复位ov9650
	writel((1 << 30) | (1 << 29) | (1 << 26), CIGCTRL);
	delayms(10);
	writel(readl(CIGCTRL) & ~(1 << 30), CIGCTRL);
	delayms(10);
}
