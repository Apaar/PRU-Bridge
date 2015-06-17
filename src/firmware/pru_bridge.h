#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include <stdint.h>


int check_init(void);
uint8_t read_buffer(int ring_no);
void write_buffer(int ring_no,uint8_t data);
void pru_bridge_init(void);

#endif
