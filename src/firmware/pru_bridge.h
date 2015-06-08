#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include pru_defs.h

#define NUM_VALUES 10


struct circular_buffer
{
	volatile int head;
	volatile int tail;
	char buffer[NUM_VALUES];
};

#define RBUFF	((volatile struct ring *)DPRAM_SHARED)

inline void write_buffer(char data)
{

    ring->buffer[ring->tail] = data;
    ring->tail = (ring->tail+1)%NUM_VALUES;
}

inline char read_buffer(void)
{
    char value = ring->buffer[ring->head];
    ring->head = (ring->head+1)%NUM_VALUES;
    return value;
}

#endif
