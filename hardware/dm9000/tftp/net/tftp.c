#include <stdio.h>
#include <string.h>
#include <net.h>

static struct tftp_package *pGtftp = NULL;
static unsigned int g_len = 0;
static unsigned short TftpServerPort = 0;

//#define TFTP_MTU_BLOCKSIZE 512
#define TFTP_MTU_BLOCKSIZE 1468
static unsigned short TftpBlkSizeOption = TFTP_MTU_BLOCKSIZE;

static void tftp_send_request(unsigned char *buf, const char *filename)
{
    int len = 0;
    buf[0] = 0x00;							//读请求
    buf[1] = 0x01;
    len += 2;
    sprintf((char*)(&buf[len]), "%s", filename);
    len += strlen(filename);
    buf[len] = '\0';
    len += 1;
    sprintf((char*)(&buf[len]), "%s", "octet");
    len += strlen("octet");
    //sprintf(&buf[len], "%s", "netascii");
    //len += strlen("netascii");
    buf[len] = '\0';
    len += 1;
    
	if(512 != TftpBlkSizeOption)
	{
		len += sprintf((char*)(&buf[len]), "timeout%c%d%c", 0, 5, 0);
		/* try for more effic. blk size */
		len += sprintf((char*)(&buf[len]), "blksize%c%d%c", 0, TftpBlkSizeOption, 0);
	}

    udp_send(buf, len, 69);					//tftp采用端口号69,协议规定的
}

static void tftp_send_ack(unsigned char *buf, unsigned short blocknum)
{
    struct tftp_package *A = (struct tftp_package *)buf;
    A->opcode = htons(0x04);									//ACK包
    A->u.blocknum = htons(blocknum);
    udp_send(buf, sizeof(struct tftp_package), TftpServerPort);
}

void tftp_handle( unsigned char *buf, unsigned int len, unsigned short port)
{
    g_len = len; 
    TftpServerPort = port;
    pGtftp = (struct tftp_package *)buf; 
}

int tftp_download(unsigned char *addr, const char *filename)
{  
    int i=0;
    unsigned short curblock = 1; //第一块数据block data

	if(is_broadcast_mac_addr(host_mac_addr))
		send_arp();

	ip_print("board ip", ip_addr);
	ip_print("server ip", host_ip_addr);
	printf("tftp file %s to addr 0x%x\r\n", filename, addr);
	
	pGtftp = NULL;
	g_len = 0;

	while(pGtftp == NULL)
	{
		i = 0;
		tftp_send_request(&send_buffer[256], filename ); //请求下载文件,为什么是从256开始,因为发送数据时要添加头部
		while(pGtftp == NULL)
		{
			eth_rx(recv_buffer);
			i++;
			if(i == 0x50000) /* 在一定的时间内还没有接收到服务器发送的tftp ack包，就重新发送tftp request. */
			{
				putc('T');
				break;
			}
		}
	}

    while (1)
    {
    	if(pGtftp == NULL)
	        eth_rx(recv_buffer);

        if( pGtftp == NULL ) //查询的方式，看有没有收到数据
            continue;
        if ( ntohs(pGtftp->opcode) == TFTP_DATA )					//收到的是数据包
        {
//			printf("*pGtftp->u.blocknum	=%d, curblock = %d g_len = %d\r\n", ntohs(pGtftp->u.blocknum), curblock, g_len);
				
            if (ntohs(pGtftp->u.blocknum) == curblock)  			//是第一个数据包，而不是ACK包
            {
//                printf("\rCurrent Block Number = %d", curblock);
				if(curblock % 32 == 1)
					putc('#');
				
                for (i=0; i<g_len-4; i++) //减4就是减去opcode和block的长度，共4个字节
                    *(addr++) = *(pGtftp->data+i);
                
//				printf("=pGtftp->u.blocknum  =%d, curblock = %d\r\n", ntohs(pGtftp->u.blocknum), curblock);
                tftp_send_ack( &send_buffer[256], curblock);
                
                if (g_len < /*TFTP_DATASIZE*/TftpBlkSizeOption+4) //一样的道理，512只是数据的长度，iGLen还包括opcode和block的长度
					 break;					//数据全部收完，退出
                   
                curblock += 1; //记录传送的数据包数量
            }
            else if (ntohs(pGtftp->u.blocknum) < curblock)	//收到的是ACK包,发一个ACK应答包
            {
#if 0            
            	printf("<pGtftp->u.blocknum  =%d, curblock = %d\r\n", ntohs(pGtftp->u.blocknum), curblock);
                tftp_send_ack( &send_buffer[256], ntohs(pGtftp->u.blocknum)); 
#endif                
            }
            else
            {
                printf("\n\rBlock Number Not Match.");                
            }
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_ERROR ) //收到的是错误包,进行错误处理
        {
            switch( ntohs(pGtftp->u.errcode) )
            {
                case ENOTFOUND:
                    printf(" Error: File NOT Found.\n\r");
                    break;
                case EBADOP:
                    printf(" Error: Bad TFTP Operation.\n\r");
                    break;
                case EBADID:
                    printf(" Error: Unknown Transfer ID.\n\r");
                    break;
                default:
                    printf(" Error: ..... \n\r");
                    break;
            } 
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_RRQ ) 
        {
            printf(" Get TFTP RRQ Packet.\n\r");
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_WRQ ) 
        {
            printf(" Get TFTP WRQ Packet.\n\r");
        }
        else if ( ntohs(pGtftp->opcode) == TFTP_ACK ) 
        {
            printf(" Get TFTP ACK Packet.\n\r");
        }
        else if ( ntohs(pGtftp->opcode) == 6 ) 
        {
//        	printf(" Opcode = %d.\n\r", htons(pGtftp->opcode));
        	tftp_send_ack( &send_buffer[256], 0); 
        }
        else 
        {
            printf(" Unknown TFTP Packet.");
            printf(" Opcode = %d.\n\r", htons(pGtftp->opcode));
        }

        pGtftp = NULL;											//进行下一次包的处理
        g_len = 0;
    }

    printf("\n\rTransfer complete: %d Bytes.\n\r", (curblock-1)*TftpBlkSizeOption + g_len-4);//iGLen是最后一次数据的长度
    return 0;
}
