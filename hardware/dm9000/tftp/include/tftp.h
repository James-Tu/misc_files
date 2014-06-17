#ifndef __TFTP_H
#define __TFTP_H

extern void tftp_handle( unsigned char *buf, unsigned int len, unsigned short port);
extern int tftp_download(unsigned char *addr, const char *filename);

#endif
