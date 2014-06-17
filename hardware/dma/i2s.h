#ifndef __I2S_H
#define __I2S_H

extern void i2s_init();
extern void play_audio(unsigned char *buf, int len);
extern void uda1341_init();
extern void uda1341_set_volume(int volume);
extern void uda1341_set_mute(int mute);
extern void record_audio(unsigned char *buf, int len);
extern void int_dma2_handle();

#endif
