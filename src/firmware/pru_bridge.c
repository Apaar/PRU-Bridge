
/*
 * PRU Bridge firmware to enable data trsfer between PRU<-->ARM
 *
 * Copyright (C) 2015 Apaar Gupta
 * This file is licensed under the terms of the GNU General Public License
 * version 3.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include <stdint.h>

#include "pru_bridge.h"
#include "pru_defs.h"
#define NUM_CHANNELS 5
#define TOTAL_BUFFER_SIZE 11500

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


int read_buffer(int ring_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        if(control_channel->index_data[ring_no] != 0)
        {
            *(pru_data+i) = ring->data[control_channel->buffer_start[ring_no] + control_channel->head[ring_no]];
            (control_channel->index_data[ring_no])--;
            control_channel->head[ring_no] = (control_channel->head[ring_no]+1)%(control_channel->channel_size[ring_no]);
        }
        else
            return -1;

        i++;
    }
    return 0;
}
int write_buffer(int ring_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        ring->data[control_channel->buffer_start[ring_no] + control_channel->tail[ring_no]] = *(pru_data+i);
        if((control_channel->index_data[ring_no]) < (control_channel->channel_size[ring_no]))     //allows pru to check if there is data to read or not
            (control_channel->index_data[ring_no])++;
        control_channel->tail[ring_no] = (control_channel->tail[ring_no]+1)%(control_channel->channel_size[ring_no]);
        i++;
    }
    return 0;
}

int check_init(void)
{
    return control_channel->init_check;
}

