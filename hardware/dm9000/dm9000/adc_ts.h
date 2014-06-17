#ifndef __ADC_TS_H
#define __ADC_TS_H

extern void adc_ts_init();
extern unsigned int read_adc(int ch);
extern void ts_init();
extern void int_adc_handle();
extern int tslib_calibrate();

#endif
