#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

int check_init(void);
char read_buffer(int ring_no);
void write_buffer(int ring_no,char data);
void pru_bridge_init(void);

#endif
