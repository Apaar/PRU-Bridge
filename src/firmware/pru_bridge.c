#include <stdint.h>

#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_CHANNELS 5


struct control_channel
{
	volatile uint16_t init_check;
	volatile uint16_t channel_size[NUM_CHANNELS];
	volatile uint16_t pru_data[NUM_CHANNELS];
	volatile uint16_t driver_data[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel = (volatile struct control_channel*)DPRAM_SHARED;

#define CONTROL_SIZE sizeof(size_control)+(sizeof(size_control)%4)

struct circular_buffer
{
	volatile uint16_t head;
	volatile uint16_t tail;
	volatile uint16_t* buffer;
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
            ring[i]->buffer =(volatile uint16_t*) (last_address + CIRCULAR_BUFFER_SIZE);
            last_address = last_address + CIRCULAR_BUFFER_SIZE + (int)(sizeof(uint16_t)*control_channel->channel_size[i]);
            i++;
        }
    }
}

void write_buffer(int ring_no,uint16_t data)
{
    *(ring[ring_no]->buffer + ring[ring_no]->tail) = (uint16_t)data;
    ring[ring_no]->tail = (ring[ring_no]->tail+1)%(control_channel->channel_size[ring_no]);
    if((control_channel->pru_data[ring_no])<(control_channel->channel_size[ring_no]))     //allows pru to check if there is data to read or not
        (control_channel->pru_data[ring_no])++;
}

uint16_t read_buffer(int ring_no)
{
    if((control_channel->pru_data[ring_no]) != 0)
    {
        uint16_t value = *(ring[ring_no]->buffer + ring[ring_no]->head);
        ring[ring_no]->head = (ring[ring_no]->head+1)%(control_channel->channel_size[ring_no]);
        (control_channel->pru_data[ring_no])--;             //allows pru to check if there is data to read or not
        return value;
    }
    else
        return -1;
}

int check_init(void)
{
    return control_channel->init_check;
}

