

#include <stdint.h>
#include <stdio.h>

#include "pru_bridge.h"

int main()
{
    int size[5] = {100,100,100,100,100};
    pru_bridge_init(size);

    pru_channel_open(1,PRU_WRITE);
    pru_channel_open(2,PRU_WRITE);
    pru_channel_open(3,PRU_WRITE);
    pru_channel_open(4,PRU_WRITE);
    pru_channel_open(5,PRU_WRITE);

    int pru_data = 10000;
    pru_write(1,(uint8_t *)&pru_data,4);
    pru_write(2,(uint8_t *)&pru_data,4);
    pru_write(3,(uint8_t *)&pru_data,4);
    pru_write(4,(uint8_t *)&pru_data,4);
    pru_write(5,(uint8_t *)&pru_data,4);


    pru_channel_close(1);
    pru_channel_close(2);
    pru_channel_close(3);
    pru_channel_close(4);
    pru_channel_close(5);
    return 0;
}
