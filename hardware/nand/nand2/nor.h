#ifndef __NOR_H
#define __NOR_H

extern void nor_read_id();
extern void nor_sector_erase(unsigned int sec);
extern void nor_program_2bytes(unsigned int cpu_addr, unsigned short data);
extern void nor_program_bytes(unsigned int cpu_addr, 
unsigned short *buf, unsigned int len);
extern void nor_cfi_query();

#endif
