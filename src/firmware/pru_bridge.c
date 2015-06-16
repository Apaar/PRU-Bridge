#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_CHANNELS 10


struct control_channel
{
	volatile int init_check;
	volatile int channel_size[NUM_CHANNELS];
	volatile int pru_data[NUM_CHANNELS];
	volatile int driver_data[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel = (volatile struct control_channel*)DPRAM_SHARED;

#define CONTROL_SIZE sizeof(size_control)

struct circular_buffer
{
	volatile int head;
	volatile int tail;
	volatile char* buffer;
}size_ring;

volatile struct circular_buffer* ring[NUM_CHANNELS];

#define CIRCULAR_BUFFER_SIZE sizeof(size_ring)

void pru_bridge_init(void)
{
    if(control_channel->init_check == 1)
    {
        int last_address,i=0;
        last_address = DPRAM_SHARED+CONTROL_SIZE;

        while(i<NUM_CHANNELS)
        {
            ring[i] = (volatile struct circular_buffer*)last_address;
            ring[i]->buffer =(volatile char*) (last_address + CIRCULAR_BUFFER_SIZE);
            last_address = last_address + CIRCULAR_BUFFER_SIZE +(sizeof(char)*control_channel->channel_size[i]);
            i++;
        }
    }
}

void write_buffer(int ring_no,char data)
{
    *(ring[ring_no]->buffer + ring[ring_no]->tail) = data;
    ring[ring_no]->tail = (ring[ring_no]->tail+1)%(control_channel->channel_size[ring_no]);
}

char read_buffer(int ring_no)
{
    char value = *(ring[ring_no]->buffer + ring[ring_no]->head);
    ring[ring_no]->head = (ring[ring_no]->head+1)%(control_channel->channel_size[ring_no]);
    return value;
}

int check_init(void)
{
    return control_channel->init_check;
}

