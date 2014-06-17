#ifndef __CMIF_H
#define __CMIF_H

extern void ov9650_init();
extern void cmif_init();
extern void cmif_config(unsigned int TargetHsize, unsigned int TargetVsize,
	unsigned int WinHorOfst, unsigned int WinVerOfst);
extern void start_capture();
extern void stop_capture();

#endif
