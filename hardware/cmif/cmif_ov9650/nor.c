#include<stdio.h>

#define flash_base 0x0		// ngcs0
static inline void nor_write(unsigned int nor_addr, unsigned short data)
{
	*((volatile unsigned short*)(flash_base + (nor_addr << 1))) = data;
}

static inline unsigned short nor_read(unsigned int nor_addr)
{
	return *((volatile unsigned short*)(flash_base + (nor_addr << 1)));
}

void nor_read_id()
{
	unsigned short manu_id, dev_id;

	nor_write(0x555, 0xaa);
	nor_write(0x2aa, 0x55);
	nor_write(0x555, 0x90);

	manu_id = nor_read(0x0);
	dev_id = nor_read(0x1);

	printf("nor flash: manu_id = 0x%x  dev_id = 0x%x\r\n", manu_id, dev_id);

	nor_write(0x0, 0xf0);	//reset nor flash
}

unsigned int nor_sec2addr(unsigned int sec)
{
	switch(sec)
	{
		case 0: return 0x0;
		case 1: return 0x02000;
		case 2: return 0x03000;
		case 3: return 0x04000;
		default: return (sec -3) * (32 * 1024 /* 32K */);
	}
}

unsigned int nor_check_toggle()
{
	unsigned short t1, t2;
	
	while(1)
	{
		t1 = nor_read(0x0);
		t2 = nor_read(0x0);

		//如果 DQ6 没有toggle， 表示操作成功
		if((t1 & 0x40) == (t2 & 0x40))
			break;

		//如果DQ5=1， 表示超时了
		if(t2 & 0x20)
		{
			t1 = nor_read(0x0);
			t2 = nor_read(0x0);

			//如果 DQ6 没有toggle， 表示操作成功
			if((t1 & 0x40) == (t2 & 0x40))
				break;
			else
			{
				nor_write(0x0, 0xf0);	//reset nor flash
				printf("nor flash error . \r\n");
				return 0;  // 出错
			}
		}
	}

	return 1;  /* 操作成功 */
}

/*
	nor flash S29AL016J:  0 <= sec <= 34
	如果sec > 34, 擦除整片flash
*/
void nor_sector_erase(unsigned int sec)
{
	nor_write(0x555, 0xaa);
	nor_write(0x2aa, 0x55);	
	nor_write(0x555, 0x80);
	nor_write(0x555, 0xaa);
	nor_write(0x2aa, 0x55);	

	if(sec < 35)
		nor_write(nor_sec2addr(sec), 0x30);	
	else
		nor_write(0x555, 0x10);	/* 擦除整片flash */

	nor_check_toggle();
}

void nor_program_2bytes(unsigned int cpu_addr, unsigned short data)
{
	nor_write(0x555, 0xaa);
	nor_write(0x2aa, 0x55);	
	nor_write(0x555, 0xa0);
	nor_write(cpu_addr >> 1, data);

	nor_check_toggle();
}

//len 必须2字节对齐
void nor_program_bytes(unsigned int cpu_addr, 
unsigned short *buf, unsigned int len)
{
	unsigned int i;

	for(i = 0; i < (len + 1) / 2; i++)
		nor_program_2bytes(cpu_addr + 2 * i, *buf++);
}

void nor_cfi_query()
{
	unsigned short addr;
	
	nor_write(0x55, 0x98);	//cfi cmd

	for(addr = 0x10; addr <= 0x50; addr++)
		printf("cfi query addr:0x%x  data:0x%x\r\n", addr, nor_read(addr));

	nor_write(0x0, 0xf0);	//reset nor flash
}

