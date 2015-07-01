#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include <stdint.h>

#define NUM_CHANNELS 5
#define PRU_READ 0
#define PRU_WRITE 1

int pru_bridge_init(int channel_sizes[NUM_CHANNELS]);
void pru_channel_open(int channel_no,int type);
void pru_channel_close(int channel_no);
int pru_write(int channel_no,uint8_t* pru_data,uint8_t length);
int pru_read(int channel_no,uint8_t* pru_data,uint8_t length);

#endif
