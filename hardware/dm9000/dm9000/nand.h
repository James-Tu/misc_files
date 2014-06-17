#ifndef __NAND_H
#define __NAND_H

extern void nand_init();
extern void nand_read_id();
extern void nand_read_page(int page, unsigned char *buf);
extern unsigned char nand_random_read(int page, unsigned int addr);
extern void nand_write_page(int page, unsigned char *buf);
extern void nand_random_write(int page, unsigned int addr, unsigned char data);
extern int is_bad_block(int block);
extern void mark_bad_block(int block);
extern void nand_erase_block(int block);
extern void nand_read_bytes(int page, unsigned char *buf, int size);
extern void nand_write_bytes(int page, unsigned char *buf, int size);

#endif
