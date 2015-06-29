#include <stdint.h>

#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_CHANNELS 5
#define TOTAL_BUFFER_SIZE 5000

struct control_channel
{
	volatile uint16_t init_check;
	volatile uint16_t channel_size[NUM_CHANNELS];
	volatile uint16_t index_data[NUM_CHANNELS];
	volatile uint16_t buffer_start[NUM_CHANNELS];
	volatile uint16_t head[NUM_CHANNELS];
	volatile uint16_t tail[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel = (volatile struct control_channel*)DPRAM_SHARED;

#define CONTROL_SIZE sizeof(size_control)+(sizeof(size_control)%4)

struct circular_buffers
{
	uint8_t data[TOTAL_BUFFER_SIZE];
};

volatile struct circular_buffers* ring = (volatile struct circular_buffers*)(DPRAM_SHARED + CONTROL_SIZE);


int write_buffer(int ring_no,uint8_t pru_data)
{
        ring->data[control_channel->buffer_start[ring_no] + control_channel->tail[ring_no]] = pru_data;

        if((control_channel->index_data[ring_no])<(control_channel->channel_size[ring_no]))     //allows pru to check if there is data to read or not
            (control_channel->index_data[ring_no])++;

        control_channel->tail[ring_no] = (control_channel->tail[ring_no]+1)%(control_channel->channel_size[ring_no]);

    return 0;
}

uint8_t read_buffer(int ring_no)
{

    if(control_channel->index_data[ring_no] == 0)
    {
        uint8_t value = ring->data[control_channel->buffer_start[ring_no] + control_channel->head[ring_no]];

        (control_channel->index_data[ring_no])--;

        control_channel->head[ring_no] = (control_channel->head[ring_no]+1)%(control_channel->channel_size[ring_no]);
        return value;
    }
    else
        return 0;
}

int check_init(void)
{
    return control_channel->init_check;
}

