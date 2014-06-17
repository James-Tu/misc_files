#include<io.h>
#include<stdio.h>
#include<regs-clock.h>
#include<regs-nand.h>
#include"clock.h"

#define CMD_READ1                 0x00              //页读命令周期1
#define CMD_READ2                0x30              //页读命令周期2
#define CMD_READID               0x90              //读ID命令
#define CMD_WRITE1               0x80              //页写命令周期1
#define CMD_WRITE2               0x10              //页写命令周期2
#define CMD_ERASE1               0x60              //块擦除命令周期1
#define CMD_ERASE2               0xd0              //块擦除命令周期2
#define CMD_STATUS	0x70	//读状态命令, IO0(Pass : "0" Fail : "1")   IO6(Busy : "0"  Ready : "1")
#define CMD_RESET                 0xff               //复位
#define CMD_RANDOMREAD1         0x05       //随机读命令周期1
#define CMD_RANDOMREAD2         0xE0       //随机读命令周期2
#define CMD_RANDOMWRITE         0x85       //随机写命令

#define NAND_PAGE_SIZE 2048
#define BAD_BLOCK_MARKER_OPS 2048
#define BLOCK_PAGE_SHIFT 6

#define nand_select() writel(readl(NFCONT) & ~(1 << 1), NFCONT)
#define nand_deselect() writel(readl(NFCONT) | (1 << 1), NFCONT)
#define nand_detect_rnb() while(!(readl(NFSTAT) & (1 << 2))) /* 等待，直到检测到rnb由低变高，即nand flash变为ready状态 */
#define nand_clear_rnb() writel(readl(NFSTAT) | (1 << 2), NFSTAT)

#define TACLS 1
#define TWRPH0 1
#define TWRPH1 0
void nand_init()
{
	//nand flash pin脚功能采用默认配置即可，软件上不用重新设置

	clk_enable(CLKCON_NAND, 1);

	writel((TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4), NFCONF);
	writel(3, NFCONT);
}

void nand_read_id()
{
	nand_select();

	writeb(CMD_READID, NFCMD);
	writeb(0x0, NFADDR);

	//K9F2G08U0B
	printf("maker:   0x%x\r\n", readb(NFDATA));		//0xec
	printf("dev:     0x%x\r\n", readb(NFDATA));		//0xda
	printf("3rd:	 0x%x\r\n", readb(NFDATA)); 	//0x10
	printf("4th:	 0x%x\r\n", readb(NFDATA)); 	//0x95
	printf("5th:	 0x%x\r\n", readb(NFDATA)); 	//0x44

	nand_deselect();
}

