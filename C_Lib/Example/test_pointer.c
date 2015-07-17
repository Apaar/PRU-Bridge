#include <stdint.h>
#include <stdio.h>

#include "pru_bridge.h"

int main()
{
    int size[5] = {1000,0,0,0,0};
    pru_bridge_init(size);
//    pru_channel_open(1,PRU_READ);
    pru_channel_open(1,PRU_WRITE);
    int write_data = 10000;
//    int pru_data;
    pru_write(1,(uint8_t *)&write_data,4);
//    pru_read(1,(uint8_t *)&pru_data,4);
//    printf("%d\n",pru_data);
    pru_channel_close(1);
//    pru_channel_close(1);
    return 0;
}
