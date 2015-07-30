
/*
 * PRU Bridge firmware to enable data trsfer between PRU<-->ARM
 *
 * Copyright (C) 2015 Apaar Gupta
 * This file is licensed under the terms of the GNU General Public License
 * version 3.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include "pru_bridge.h"

#define NUM_CHANNELS 10
#define TOTAL_BUFFER_SIZE 11500
#define SHARED_MEM	0x00010000

static volatile uint32_t* init_check = (volatile uint32_t*)SHARED_MEM;
static volatile uint32_t* channel_size = (volatile uint32_t*)(SHARED_MEM + 4);
static volatile uint32_t* index_data = (volatile uint32_t*)(SHARED_MEM + 4 + NUM_CHANNELS*4);
static volatile uint32_t* buffer_start = (volatile uint32_t*)(SHARED_MEM + 4 + NUM_CHANNELS*8);
static volatile uint32_t* head = (volatile uint32_t*)(SHARED_MEM + 4 + NUM_CHANNELS*12);
static volatile uint32_t* tail = (volatile uint32_t*)(SHARED_MEM + 4 + NUM_CHANNELS*16);

#define CONTROL_SIZE 4 + NUM_CHANNELS*4*5

static volatile	uint8_t* data = (volatile uint8_t*)SHARED_MEM + CONTROL_SIZE;

int read_buffer(int ring_no,uint8_t* pru_data,int length)
{
    if(((int)index_data[ring_no] - length) >= 0)
    {
        int i;
        for (i = 0 ; i < length ; i++)
        {
            *(pru_data+i) = data[buffer_start[ring_no] + head[ring_no]];
            head[ring_no] = (head[ring_no]+1)%(channel_size[ring_no]); 
        }
	index_data[ring_no] -= length;
    }
    else
	return -1;

            
    return 0;
}
int write_buffer(int ring_no,uint8_t* pru_data,int length)
{
    int i;
    for (i = 0 ; i < length ; i++)
    {
        data[buffer_start[ring_no] + tail[ring_no]] = *(pru_data+i);
        tail[ring_no] = (tail[ring_no]+1)%(channel_size[ring_no]);
    }
    if((index_data[ring_no] + length) < (channel_size[ring_no]))     //allows pru to check if there is data to read or not
        index_data[ring_no] += length;
    else
        index_data[ring_no] = channel_size[ring_no];

    return 0;
}

int check_init(void)
{
    return *init_check;
}

int check_index(int ring_no)
{
    return ((channel_size[ring_no]) - (index_data[ring_no]));
}
