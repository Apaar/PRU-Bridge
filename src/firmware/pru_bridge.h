#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

void init_buffer(void);
void write_buffer(char data);
char read_buffer(void);

#endif
