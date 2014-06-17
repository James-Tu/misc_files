#include <string.h>
#include <net.h>
#include <tftp.h>
#include <stdio.h>

/* �����������mac��ַ��ip��ַ */
unsigned char mac_addr[6]={0x11,0x22,0x33,0x44,0x55,0x66};
unsigned char ip_addr[4]={192,168,1,24};

/* ������������mac��ַ��ip��ַ */
unsigned char host_ip_addr[4]={192,168,1,14};
unsigned char host_mac_addr[6]={0xff,0xff,0xff,0xff,0xff,0xff};

/* 
	�ο���̫��Ethernet V2����֡��ʽ
	һ�����ݰ��1518(Ŀ�ĵ�ַ(6), Դ��ַ(6),����(2),����(46-1500)�� crc(4))�� ���ᳬ��2048���ֽ�
*/
unsigned char send_buffer[2048];
unsigned char recv_buffer[2048];

void mac_print(char *info, unsigned char *mac)
{
	printf("%s: %02x-%02x-%02x-%02x-%02x-%02x\r\n", info, 
			mac[0], mac[1], mac[2], mac[3],
			mac[4], mac[5]);
}

void ip_print(char *info, unsigned char *ip)
{
	printf("%s: %d.%d.%d.%d\r\n", info, ip[0], ip[1], ip[2], ip[3]);
}

unsigned short checksum(unsigned char *ptr, int len)
{
    unsigned long sum = 0;
    unsigned short *p = (unsigned short *)ptr;

    while (len > 1)
    {
        sum += *p++;
        len -= 2;
    }
    
    if(len == 1)
        sum += *(unsigned char *)p;
    
    while(sum>>16)
        sum = (sum&0xffff) + (sum>>16);
    
    return (unsigned short)((~sum)&0xffff);
}

void mac_send( unsigned char *buf, unsigned int len, unsigned short proto)
{
    buf -= sizeof(struct eth_hdr);
    len += sizeof(struct eth_hdr);
    
    struct eth_hdr *p = (struct eth_hdr *)(buf);
    
    memcpy (p->s_mac, mac_addr, 6);
    memcpy (p->d_mac, host_mac_addr, 6); 
    p->type = htons(proto);
     
    eth_send( buf, len );
}

void ip_send( unsigned char *buf, unsigned int len )
{
    buf -= sizeof(struct ip_hdr);					//����IPͷ��
    len += sizeof(struct ip_hdr);
    
    struct ip_hdr *p = (struct ip_hdr*)(buf);
    
    p->vhl = 0x45;                         				 
    p->tos = 0x00;                              		
    p->len = htons(len);       							
    p->ipid = htons(0x00);           					
    p->ipoffset = htons(0x4000);               
    p->ttl = 0xFF;                             
    p->proto = 17;        							//�ϲ�Э�����UDP                  	
	memcpy(p->srcipaddr,ip_addr,4);
	memcpy(p->destipaddr,host_ip_addr,4);
    p->ipchksum = 0x0;                          
    p->ipchksum = checksum( buf, sizeof(struct ip_hdr) );
    mac_send( buf, len, PROTO_IP );
}

void udp_send( unsigned char *buf, unsigned int len, unsigned short port )
{
    buf -= sizeof(struct udp_hdr);						//����udpͷ��
    len += sizeof(struct udp_hdr);

	struct udp_hdr *p = (struct udp_hdr*)(buf);
    p->sport  = htons(0x3000);    						// 2 Byte,suppose we use 0x8da4 port
    p->dport = htons(port);   							// 2 Byte
    p->len = htons(len);       							// 2 Byte 
    p->udpchksum  = 0x00;     							// Do Not Checksum, 2 Byte 

    ip_send( buf, len );
}

void arp_request(void) 								//send a ARP request packet
{
	struct arp_hdr *arpbuf;
	arpbuf = (struct arp_hdr *)&send_buffer[256];
	
	arpbuf->hwtype = htons( 1 );					//the type of send hardware address,if 1 means yi tai wang di zhi
	arpbuf->protocol = htons( 0x0800 );				//means ip------>arp
	arpbuf->hwlen = 6;
	arpbuf->protolen = 4;
	arpbuf->opcode = htons( 1 );					//arp request
	memcpy(arpbuf->smac, mac_addr, 6);
	memcpy(arpbuf->sipaddr, ip_addr, 4);
	memcpy(arpbuf->dipaddr, host_ip_addr, 4);
	mac_send(&send_buffer[256], sizeof(struct arp_hdr)/* =28 */, PROTO_ARP );
}

static int udp_handle( unsigned char *buf, unsigned int len )
{
    struct udp_hdr *p;
    p = (struct udp_hdr *)buf;
       
    buf += sizeof(struct udp_hdr);
    len -= sizeof(struct udp_hdr);

    tftp_handle( buf, len, htons(p->sport) );
    
    return 0;
}

unsigned long ip_handle(unsigned char *buf, unsigned int len)
{
	struct ip_hdr *p;
	p = (struct ip_hdr *)buf;    
		
	if((p->destipaddr[0] != ip_addr[0]) || \
	   (p->destipaddr[1] != ip_addr[1]) || \
	   (p->destipaddr[2] != ip_addr[2]) || \
	   (p->destipaddr[3] != ip_addr[3]))
	   {
	   		//printf("it is not my ip packet!\n\r");
	   		return -1;
	   }
	   
	   if(p->vhl != 0x45)
	   	{	
	   		printf("it is not a IPV4 packet!\n\r");
	   		return -1;
	   	}
	   	
    switch ( p->proto )
    {	
    	
        case PROTO_UDP:
            buf += sizeof(struct ip_hdr);
            len -= sizeof(struct ip_hdr);
            udp_handle( buf, len );
            break;
        case PROTO_TCP:
        	printf("we don't handle the TCP packets!\n\r");
            break;
        default:
        	printf("IP_PROTO NOT MACTHED\n\r");
            break;
    }

    return 0;   	
}

void send_arp()
{
	int i;

	while(is_broadcast_mac_addr(host_mac_addr))
	{
		arp_request();
		
		i = 0;
		while(is_broadcast_mac_addr(host_mac_addr))
		{
			eth_rx(recv_buffer);
			i++;
			if(i == 0x50000) /* ��һ����ʱ���ڻ�û�н��յ����������͵� ack���������·���arp request. */
			{
				putc('A');
				break;
			}
		}
	}
	printf("\r\n");
}

void arp_handle(unsigned char *buf, unsigned int len)
{
	struct arp_hdr *pRx, *pTx;	
    pRx = (struct arp_hdr *)(buf);
    pTx = (struct arp_hdr *)&send_buffer[512];

	if(len<28)
	{
		printf("\narp packet length is not right!\n\r");
		return;
	}
	switch (htons(pRx->opcode))
	{
		case 1: //����ARP����
			if((pRx->dipaddr[0] == ip_addr[0]) && \
			   (pRx->dipaddr[1] == ip_addr[1]) && \
			   (pRx->dipaddr[2] == ip_addr[2]) && \
			   (pRx->dipaddr[3] == ip_addr[3]) )
			   {
//			 	printf("send mac to server ... \r\n");
			   	pTx->hwtype = htons(1);
				pTx->protocol = htons(0x0800); //����IPЭ��
				pTx->hwlen = 6;
				pTx->protolen = 4;
				pTx->opcode = htons(2); //ARPӦ����
				memcpy(pTx->sipaddr,ip_addr,4); //�����ǵ�IP��MAC��ַ��������
				memcpy(pTx->smac,mac_addr,6);
				memcpy(pTx->dipaddr,pRx->sipaddr,4); //���յ���Դ����IP��ַ��MAC��ַ��ΪĿ���ַ
			   	memcpy(pTx->dmac, pRx->smac,6);

			   	mac_send( (unsigned char*)pTx, sizeof(struct arp_hdr), PROTO_ARP);
   				}
			   break;
		case 2: //����arpӦ��
			if((pRx->dipaddr[0] == ip_addr[0]) && \
			   (pRx->dipaddr[1] == ip_addr[1]) && \
			   (pRx->dipaddr[2] == ip_addr[2]) && \
			   (pRx->dipaddr[3] == ip_addr[3]) )
			   			   
			   memcpy(host_mac_addr,pRx->smac,6);
//				mac_print("server mac", host_mac_addr);
			   break;
		default:
				printf("\n\r arp_opcode Not Support.\n");
				break;
	}
}


void net_receive( unsigned char *buf, unsigned int len )
{
    struct eth_hdr *p = (struct eth_hdr *)(buf);

	len -= 4; /* ���ܵ������ݰ����4���ֽ�Ϊcrc�������ȥcrc�ĳ��� */
//	printf("frame type:0x%x     len = %d\r\n", htons(p->type), len);
    switch ( htons(p->type) )
    {
        case PROTO_ARP:
            buf += sizeof(struct eth_hdr);
            len -= sizeof(struct eth_hdr);
            arp_handle( buf, len );
            break;
        case PROTO_IP:
            buf += sizeof(struct eth_hdr);
            len -= sizeof(struct eth_hdr);
            ip_handle( buf, len ); 
            break;
        default:
//            printf("\r\tMAC_opcode type error (=0x%04X)!\n", p->type);
            break;
    }
}
