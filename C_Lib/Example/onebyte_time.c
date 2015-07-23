
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "pru_bridge.h"
#define NUM_SAMPLES 11500

int main()
{
    int size[5] = {NUM_SAMPLES,2,0,0,0};
    pru_bridge_init(size);
    pru_channel_open(1,PRU_READ);
    pru_channel_open(2,PRU_WRITE);

    int i=0;
    clock_t t1, t2;
    uint8_t pru_data = 'a';
    pru_write(2,&pru_data,1);
    t1 = clock();
    while(i<NUM_SAMPLES)
    {
        pru_read(1,(uint8_t *)(&pru_data),1);
        i++;
    }
    t2 = clock();
    double diff = ((double)(t2 - t1)) / CLOCKS_PER_SEC;
    printf("Time per byte : %f\n",diff/NUM_SAMPLES);
    pru_channel_close(1);
    pru_channel_close(2);
    return 0;
}


