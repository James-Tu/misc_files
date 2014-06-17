#include<io.h>
#include<stdio.h>
#include<regs-gpio.h>
#include<regs-i2s.h>
#include<regs-dma.h>
#include<regs-clock.h>
#include"common.h"
#include"clock.h"

#define I2S_USING_DMA

/*
	L3MODE  GPB2
	L3DATA  GPB3
	L3CLOCK GPB4
*/
#define L3MODE_H()		writel(readl(GPBDAT) | (1 << 2), GPBDAT)
#define L3MODE_L()		writel(readl(GPBDAT) & ~(1 << 2), GPBDAT)
#define L3DATA_H()		writel(readl(GPBDAT) | (1 << 3), GPBDAT)
#define L3DATA_L()		writel(readl(GPBDAT) & ~(1 << 3), GPBDAT)
#define L3CLOCK_H()		writel(readl(GPBDAT) | (1 << 4), GPBDAT)
#define L3CLOCK_L()		writel(readl(GPBDAT) & ~(1 << 4), GPBDAT)
#define l3_delay()		delayms(1)

#define UDA1341_L3ADDR		5
#define UDA1341_DATA0_ADDR		((UDA1341_L3ADDR << 2) | 0)
#define UDA1341_DATA1_ADDR		((UDA1341_L3ADDR << 2) | 1)
#define UDA1341_STATUS_ADDR		((UDA1341_L3ADDR << 2) | 2)

#define UDA1341_EXT_ADDR(a)		(0xc0 | (a))
#define UDA1341_EXT_DATA(d)		(0xe0 | (d))

void i2s_init()
{
	//i2s�ӿڳ�ʼ��
	writel((readl(GPECON) & ~0x3ff) | 0x2aa, GPECON); /* ����GPE0-4����Ϊi2s���� */
	writel(readl(GPEUP) | 0x1f, GPEUP);  /* GPE0-4������Ч */

	clk_enable(CLKCON_IIS, 1);

	//L3�ӿڳ�ʼ��
	writel((readl(GPBCON) & ~(0x3f << 4)) | (0x15 << 4), GPBCON); /* ����GPB2,3,4Ϊ��� */
	writel(readl(GPBUP) | (0x7 << 2), GPBUP); /* GPB2,3,4������Ч */

	L3MODE_H();
	L3CLOCK_H();
}

static volatile int i2ssdo_dma_remainder;
static volatile int i2ssdo_dma_times;
static volatile int play_state;

void play_audio(unsigned char *buf, int len)
{
#ifdef I2S_USING_DMA
	int tc;
	
	//��ʼ��i2s
	writel((1 << 5) | (1 << 2) | (1 << 1), IISCON); //tx dma enable, tx Not idle, rx Idle, IIS prescaler enable
	//tx mode, ÿ������16bit��codecʱ�� 384fs�� SCLKʱ�� 32fs
	writel((2 << 6) | (1 << 3) | (1 << 2) | 1, IISMOD);

	/*
		���� fs = 44.1KHZ   codecʱ��=384fs=16.9344MHZ
		codecʱ�� = PCLK / IPSR_B,     IPSR_B = 3
	*/
	writel((2 << 5) | 2, IISPSR);
	writel((1 << 15) | (1 << 13), IISFCON); //tx fifo enable, dma mode

	//dma��ʼ��
	writel((unsigned int)buf, DISRC2);
	writel(0x0, DISRCC2); // AHB, address increment
	writel(IISFIFO, DIDST2);
	writel(3, DIDSTC2); // APB, address Fixed

	i2ssdo_dma_remainder = (len / 2) & 0xfffff; /* ���һ��dma�����TC��ֵ */
	i2ssdo_dma_times = (len / 2) / 0x100000; /* ����Ҫdma������ٴ� */

	tc = i2ssdo_dma_times ? 0xfffff : i2ssdo_dma_remainder;

	/*
		Handshake mode, CURR_TC (terminal count)�ж�ʹ��,H/W request mode, reload off
		Half word
	*/
	writel((1 << 31) | (1 << 29) | (1 << 23) | (1 << 22) | (1 << 20) | tc, DCON2);
	writel(1 << 1, DMASKTRIG2); // DMA channel on
	writel(readl(IISCON) | 0x01, IISCON); //enable i2s
	play_state = 1;
#else
	int i, n;
	unsigned short* b = (unsigned short*)buf;
	
	writel((1 << 2) | (1 << 1), IISCON); //tx Not idle, rx Idle, IIS prescaler enable
	//tx mode, ÿ������16bit��codecʱ�� 384fs�� SCLKʱ�� 32fs
	writel((2 << 6) | (1 << 3) | (1 << 2) | 1, IISMOD);

	/*
		���� fs = 44.1KHZ   codecʱ��=384fs=16.9344MHZ
		codecʱ�� = PCLK / IPSR_B,     IPSR_B = 3
	*/
	writel((2 << 5) | 2, IISPSR);
	writel(1 << 13, IISFCON); //tx fifo enable

	writel(readl(IISCON) | 0x01, IISCON); //enable i2s

	while(len > 0)
	{
		while(readl(IISCON) & (1 << 7)); /* �ȴ���ֱ��tx fifoΪ�� */

		n = (len > 64)? 64 : len;
		for(i = 0; i < n / 2; i++)
			writew(*b++, IISFIFO); /* ��fifo��д���� */

		len -= n;
	}
	
	writel(0, IISCON); // disable i2s
#endif	
}

void int_dma2_handle()
{
	printf("irq dma2: i2ssdo_dma_times = %d\r\n", i2ssdo_dma_times);
	if(i2ssdo_dma_times)
	{
		//��û������ɣ���Ҫdma������������
		i2ssdo_dma_times--;
		writel(readl(DISRC2) + 0x200000, DISRC2);
		
		if(0 == i2ssdo_dma_times)
			writel((readl(DCON2) & ~0xfffff) | i2ssdo_dma_remainder, DCON2)	;
			
		writel(1 << 1, DMASKTRIG2); // DMA channel on
	}
	else
	{
		writel(0, IISCON); // disable i2s
		writel(1 << 2, DMASKTRIG2); // stop DMA
		play_state = 0;
	}
}

void record_audio(unsigned char *buf, int len)
{
	int i, n;
	unsigned short* b = (unsigned short*)buf;
	
	writel((1 << 3) | (1 << 1), IISCON); //rx Not idle, tx Idle, IIS prescaler enable
	//rx mode, ÿ������16bit��codecʱ�� 384fs�� SCLKʱ�� 32fs
	writel((1 << 6) | (1 << 3) | (1 << 2) | 1, IISMOD);

	/*
		���� fs = 44.1KHZ   codecʱ��=384fs=16.9344MHZ
		codecʱ�� = PCLK / IPSR_B,     IPSR_B = 3
	*/
	writel((2 << 5) | 2, IISPSR);
	writel(1 << 12, IISFCON); //rx fifo enable

	writel(readl(IISCON) | 0x01, IISCON); //enable i2s

	while(len > 0)
	{
		while(readl(IISCON) & (1 << 6)); /* �ȴ���ֱ��rx fifoΪ�� */

		n = (len > 64)? 64 : len;
		for(i = 0; i < n / 2; i++)
			*b++ = readw(IISFIFO); /* ��fifo�ж�ȡ���� */

		len -= n;
	}
	
	writel(0, IISCON); // disable i2s
}

/*
	flag = 1: ��ַģʽ
	flag = 0: ����ģʽ
*/
void l3_write(unsigned char v, int flag)
{
	int i;
	
	if(flag)
		L3MODE_L();

	l3_delay();

	for(i = 0; i < 8; i++)
	{
		L3CLOCK_L();
		if(v & 0x1)
			L3DATA_H();
		else
			L3DATA_L();
			
		l3_delay();
		L3CLOCK_H();
		l3_delay();
		v = v >> 1;
	}

	L3CLOCK_H();
	L3MODE_H();
}

void l3_write_addr(unsigned char addr)
{
	l3_write(addr, 1);
}

void l3_write_data(unsigned char data)
{
	l3_write(data, 0);
}

void uda1341_init()
{
	l3_write_addr(UDA1341_STATUS_ADDR);
	l3_write_data((1 << 6) | (1 << 4) | 1); // reset, 384fs, I2S-bus, DC-filtering
	delayms(1);
	l3_write_data((1 << 4) | 1); //384fs, I2S-bus, DC-filtering
	l3_write_data((1 << 7) | 3); //ADC on, DAC on

	//ѡ��¼��ͨ��2
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data(UDA1341_EXT_ADDR(2));
	l3_write_data(UDA1341_EXT_DATA(2));
}

/*
	1 <= volume <= 62
	1:  ������С
	62: �������
*/
void uda1341_set_volume(int volume)
{
	printf("volume = %d\r\n", volume);
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data(63 - volume);
}

//mute=0 ������           mute=1 ����
void uda1341_set_mute(int mute)
{
	l3_write_addr(UDA1341_DATA0_ADDR);
	l3_write_data((1 << 7) | (mute << 2));
}