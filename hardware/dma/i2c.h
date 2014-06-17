#ifndef __I2C_H
#define __I2C_H

extern void i2c_init();
extern int i2c_write(unsigned char slave_addr, unsigned char *buf, int len);
extern int i2c_read(unsigned char slave_addr, unsigned char *buf, int len);

#endif
