#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include <stdio.h>


int pru_bridge_init(int channel_sizes[NUM_CHANNELS]);
void pru_channel_open(int channel_no,int type);
void pru_channel_close(int channel_no);
int pru_write(int channel_no,uint8_t* data,uint8_t length);
int pru_read(int channel_no,uint8_t* data,uint8_t length);

#endif
