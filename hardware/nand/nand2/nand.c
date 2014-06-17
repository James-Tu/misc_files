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
void mark_bad_block(int block);

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

void nand_read_page(int page, unsigned char *buf)
{
	int page_size = NAND_PAGE_SIZE;
	
	nand_select();
	nand_clear_rnb();

	writeb(CMD_READ1, NFCMD);

	//5个地址周期
	writeb(0x0, NFADDR);	// 列地址 A0 - A7
	writeb(0x0, NFADDR);	// 列地址 A8 - A11
	writeb(page & 0xff, NFADDR);	// 行地址 A12 - A19
	writeb((page >> 8) & 0xff, NFADDR);	// 行地址 A20 - A27
	writeb((page >> 16) & 0x1, NFADDR);	// 行地址 A28

	writeb(CMD_READ2, NFCMD);

	nand_detect_rnb();

	while(page_size--)
		*buf++ = readb(NFDATA);

	nand_deselect();
}

//随机读一个字节
unsigned char nand_random_read(int page, unsigned int addr)
{
	unsigned char data;
	
	nand_select();
	nand_clear_rnb();

	writeb(CMD_READ1, NFCMD);

	//5个地址周期
	writeb(0x0, NFADDR);	// 列地址 A0 - A7
	writeb(0x0, NFADDR);	// 列地址 A8 - A11
	writeb(page & 0xff, NFADDR);	// 行地址 A12 - A19
	writeb((page >> 8) & 0xff, NFADDR);	// 行地址 A20 - A27
	writeb((page >> 16) & 0x1, NFADDR);	// 行地址 A28

	writeb(CMD_READ2, NFCMD);

	nand_detect_rnb();

	writeb(CMD_RANDOMREAD1, NFCMD);
	writeb(addr & 0xff, NFADDR);	// 列地址 A0 - A7
	writeb((addr >> 8) & 0xf, NFADDR);	// 列地址 A8 - A11
	writeb(CMD_RANDOMREAD2, NFCMD);

	data = readb(NFDATA);

	nand_deselect();
	return data;
}

void nand_write_page(int page, unsigned char *buf)
{
	int page_size = NAND_PAGE_SIZE;
	unsigned char status;
	
	nand_select();
	nand_clear_rnb();

	writeb(CMD_WRITE1, NFCMD);

	//5个地址周期
	writeb(0x0, NFADDR);	// 列地址 A0 - A7
	writeb(0x0, NFADDR);	// 列地址 A8 - A11
	writeb(page & 0xff, NFADDR);	// 行地址 A12 - A19
	writeb((page >> 8) & 0xff, NFADDR); // 行地址 A20 - A27
	writeb((page >> 16) & 0x1, NFADDR); // 行地址 A28

	while(page_size--)
		writeb(*buf++, NFDATA);

	writeb(CMD_WRITE2, NFCMD);

	nand_detect_rnb();

	writeb(CMD_STATUS, NFCMD);
	status = readb(NFDATA);

	nand_deselect();

	if(status & 0x01)
	{
		printf("nand_write_page error. page = %d status = 0x%x\r\n", page, status);
		mark_bad_block(page >> BLOCK_PAGE_SHIFT);
	}
}

//随机写一个字节
void nand_random_write(int page, unsigned int addr, unsigned char data)
{
	unsigned char status;
	
	nand_select();
	nand_clear_rnb();

	writeb(CMD_WRITE1, NFCMD);

	//5个地址周期
	writeb(0x0, NFADDR);	// 列地址 A0 - A7
	writeb(0x0, NFADDR);	// 列地址 A8 - A11
	writeb(page & 0xff, NFADDR);	// 行地址 A12 - A19
	writeb((page >> 8) & 0xff, NFADDR); // 行地址 A20 - A27
	writeb((page >> 16) & 0x1, NFADDR); // 行地址 A28

	writeb(CMD_RANDOMWRITE, NFCMD);
	writeb(addr & 0xff, NFADDR);	// 列地址 A0 - A7
	writeb((addr >> 8) & 0xf, NFADDR);	// 列地址 A8 - A11
	writeb(data, NFDATA);

	writeb(CMD_WRITE2, NFCMD);
	
	nand_detect_rnb();

	writeb(CMD_STATUS, NFCMD);
	status = readb(NFDATA);

	nand_deselect();

	if(status & 0x01)
	{
		printf("nand_random_write error. page = %d addr = %d status = 0x%x\r\n", page, addr, status);
//		mark_bad_block(page >> BLOCK_PAGE_SHIFT);
	}	
}

int is_bad_block(int block)
{
	unsigned char data = nand_random_read(block << BLOCK_PAGE_SHIFT, BAD_BLOCK_MARKER_OPS);

	if(0xff == data)
		return 0; //不是坏块
	else
		return 1;	//是坏块
}

void mark_bad_block(int block)
{
	nand_random_write(block << BLOCK_PAGE_SHIFT, BAD_BLOCK_MARKER_OPS, 0x44);
}

void nand_erase_block(int block)
{
	int page = block << BLOCK_PAGE_SHIFT;
	unsigned char status;
	
	nand_select();
	nand_clear_rnb();

	writeb(CMD_ERASE1, NFCMD);

	//写入3个行地址周期，从A12开始
	writeb(page & 0xff, NFADDR);	// 行地址 A12 - A19
	writeb((page >> 8) & 0xff, NFADDR); // 行地址 A20 - A27
	writeb((page >> 16) & 0x1, NFADDR); // 行地址 A28

	writeb(CMD_ERASE2, NFCMD);

	nand_detect_rnb();

	writeb(CMD_STATUS, NFCMD);
	status = readb(NFDATA);

	nand_deselect();

	if(status & 0x01)
	{
		printf("nand_erase_block error. block = %d status = 0x%x\r\n", block, status);
		mark_bad_block(block);
	}
}

void nand_read_bytes(int page, unsigned char *buf, int size)
{
	int i;

	for(i = 0; i < size; i = i + NAND_PAGE_SIZE)
	{
		while(1)
		{
			if(is_bad_block(page >> BLOCK_PAGE_SHIFT))
			{
				//如果是bad block， 跳过当前block，page等于下一个block的起始页
				page = ((page >> BLOCK_PAGE_SHIFT) + 1) << BLOCK_PAGE_SHIFT;
			}
			else
				break;
		}
		nand_read_page(page, buf + i);
		page++;
	}
}

void nand_write_bytes(int page, unsigned char *buf, int size)
{
	int i;

	for(i = 0; i < size; i = i + NAND_PAGE_SIZE)
	{
		while(1)
		{
			if(is_bad_block(page >> BLOCK_PAGE_SHIFT))
			{
				//如果是bad block， 跳过当前block，page等于下一个block的起始页
				page = ((page >> BLOCK_PAGE_SHIFT) + 1) << BLOCK_PAGE_SHIFT;
			}
			else
				break;
		}
		nand_write_page(page, buf + i);
		page++;
	}
}
