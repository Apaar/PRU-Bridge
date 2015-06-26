#include <stdint.h>

#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_CHANNELS 5


struct control_channel
{
	volatile uint16_t init_check;
	volatile uint16_t channel_size[NUM_CHANNELS];
	volatile uint16_t pru_data[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel = (volatile struct control_channel*)DPRAM_SHARED;

#define CONTROL_SIZE sizeof(size_control)+(sizeof(size_control)%4)

struct circular_buffer
{
	volatile uint16_t head;
	volatile uint16_t tail;
	volatile uint8_t* buffer;
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
            ring[i]->buffer =(volatile uint8_t*) (last_address + CIRCULAR_BUFFER_SIZE);
            last_address = last_address + CIRCULAR_BUFFER_SIZE + (int)(sizeof(uint8_t)*control_channel->channel_size[i]);
            i++;
        }
    }
}

int write_buffer(int ring_no,void* prudata,uint8_t length)
{
    int i = 0;
    uint8_t *data = (uint8_t*)prudata;
    while(i<length)
    {
        *(ring[ring_no]->buffer + ring[ring_no]->tail) = *(data+i);
        ring[ring_no]->tail = (ring[ring_no]->tail+1)%(control_channel->channel_size[ring_no]);
        if((control_channel->pru_data[ring_no])<(control_channel->channel_size[ring_no]))     //allows pru to check if there is data to read or not
            {(control_channel->pru_data[ring_no])++;}
        i++;
    }
    return length;
}

int read_buffer(int ring_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    uint8_t data[length];
    pru_data = data;
    while(i<length)
    {
        if((control_channel->pru_data[ring_no]) != 0)
        {
            data[i] = *(ring[ring_no]->buffer + ring[ring_no]->head);
            ring[ring_no]->head = (ring[ring_no]->head+1)%(control_channel->channel_size[ring_no]);
            (control_channel->pru_data[ring_no])--;             //allows pru to check if there is data to read or not
        }
        else
            return -1;

        i++;
    }
    return length;
}

int check_init(void)
{
    return control_channel->init_check;
}

