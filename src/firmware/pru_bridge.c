#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_VALUES 10

#define  ring  ((volatile struct circular_buffer *)DPRAM_SHARED)

struct circular_buffer
{
        volatile int head;
        volatile int tail;
        char buffer[NUM_VALUES];
};

void init_buffer(void)
{
	ring->head = 0;
	ring->tail = 0;
}
void write_buffer(char data)
{
    ring->buffer[ring->tail] = data;
    ring->tail = (ring->tail+1)%NUM_VALUES;
}

char read_buffer(void)
{
    char value = ring->buffer[ring->head];
    ring->head = (ring->head+1)%NUM_VALUES;
    return value;
}

