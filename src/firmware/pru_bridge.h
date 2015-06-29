#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include <stdint.h>


int check_init(void);
uint8_t read_buffer(int ring_no);
int write_buffer(int ring_no,uint8_t pru_data);
int read_buffer(int ring_no,uint8_t* pru_data,uint8_t length);
int write_buffer(int ring_no,uint8_t* pru_data,uint8_t length);
#endif
