#include<io.h>
#include<stdio.h>
#include<regs-gpio.h>
#include<regs-clock.h>
#include<regs-cmif.h>
#include"clock.h"
#include"common.h"
#include"i2c.h"
#include"ov9650.h"
#include"lcd.h"

#define SourceHsize 640
#define SourceVsize 480

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

	writel((1 << 31) | (SourceHsize << 16) | SourceVsize, CISRCFMT);
	//cmif reset
	writel(readl(CIGCTRL) | (1 << 31), CIGCTRL);
	delayms(10);
	writel(readl(CIGCTRL) & ~(1 << 31), CIGCTRL);
	delayms(10);
}

void cmif_config(unsigned int TargetHsize, unsigned int TargetVsize,
	unsigned int WinHorOfst, unsigned int WinVerOfst)
{
	unsigned int WinOfsEn;
	unsigned int mainBurstSize, remainedBurstSize;
	unsigned int H_Shift, V_Shift, PreHorRatio, PreVerRatio, MainHorRatio, MainVerRatio;
	unsigned int ScaleUp_H, ScaleUp_V;
	unsigned int SRC_Width, SRC_Height;
	
	writel((1 << 31) | (SourceHsize << 16) | SourceVsize, CISRCFMT);

	//判断是否需要使用剪切功能
	if(0 == WinHorOfst && 0 == WinVerOfst)
		WinOfsEn = 0;
	else
		WinOfsEn = 1;
	writel((WinOfsEn << 31) | (WinHorOfst << 16) | WinVerOfst, CIWDOFST);

	SRC_Width = SourceHsize - (2 * WinHorOfst);
	SRC_Height = SourceVsize - (2 * WinVerOfst);

	//判断是放大还是缩小
	if(SRC_Width >= TargetHsize)
		ScaleUp_H = 0;	//down
	else
		ScaleUp_H = 1; //up

	if(SRC_Height >= TargetVsize)
		ScaleUp_V = 0;	//down
	else
		ScaleUp_V = 1; //up

	writel((1 << 29) | (1 << 26), CIGCTRL);

	//从camera采集到的数据帧经preview通道处理之后直接放到lcd_buf
	writel((unsigned int)lcd_buf, CIPRCLRSA1);
	writel((unsigned int)lcd_buf, CIPRCLRSA2);	
	writel((unsigned int)lcd_buf, CIPRCLRSA3);
	writel((unsigned int)lcd_buf, CIPRCLRSA4);

	//设置目标图像格式
	writel((TargetHsize << 16) | TargetVsize, CIPRTRGFMT);

	//设置DMA控制相关寄存器
 	CalculateBurstSize(TargetHsize * 2, &mainBurstSize, &remainedBurstSize);
 	writel((mainBurstSize << 19) | (remainedBurstSize << 14), CIPRCTRL);

	CalculatePrescalerRatioShift(SRC_Width, TargetHsize, &PreHorRatio, &H_Shift);
	CalculatePrescalerRatioShift(SRC_Height, TargetVsize, &PreVerRatio, &V_Shift);
	MainHorRatio = (SRC_Width << 8) / (TargetHsize << H_Shift);
	MainVerRatio = (SRC_Height << 8) / (TargetVsize << V_Shift);

	//设置SCALER相关寄存器
	writel(((10 - H_Shift - V_Shift) << 28) | (PreHorRatio << 16)
			| PreVerRatio, CIPRSCPRERATIO);
	writel(((SRC_Width / PreHorRatio) << 16) | (SRC_Height / PreVerRatio), CIPRSCPREDST);
	writel((1 << 31) | (ScaleUp_H << 29) | (ScaleUp_V << 28)
			| (MainHorRatio << 16) | MainVerRatio, CIPRSCCTRL);

	writel(TargetHsize * TargetVsize, CIPRTAREA);
}

void start_capture()
{
	writel(readl(CIPRSCCTRL) | (1 << 15), CIPRSCCTRL);	// Preview scaler start
	// global capture enable, Capture enable for preview scaler
	writel(readl(CIIMGCPT) | (1 << 31) | (1 << 29), CIIMGCPT);
}

void stop_capture()
{
	// global capture disable, Capture disable for preview scaler
	writel(readl(CIIMGCPT) & ~((1 << 31) | (1 << 29)), CIIMGCPT);
	writel(readl(CIPRSCCTRL) & ~(1 << 15), CIPRSCCTRL);	// Preview scaler stop
}

