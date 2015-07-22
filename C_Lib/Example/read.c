

#include <stdint.h>
#include <stdio.h>

#include "pru_bridge.h"

int main()
{
    int size[5] = {100,100,100,100,100};
    pru_bridge_init(size);

    pru_channel_open(1,PRU_READ);
    pru_channel_open(2,PRU_READ);
    pru_channel_open(3,PRU_READ);
    pru_channel_open(4,PRU_READ);
    pru_channel_open(5,PRU_READ);

    int pru_data;
    pru_read(1,(uint8_t *)&pru_data,4);
    printf("%d\n",pru_data);
    pru_read(2,(uint8_t *)&pru_data,4);
    printf("%d\n",pru_data);
    pru_read(3,(uint8_t *)&pru_data,4);
    printf("%d\n",pru_data);
    pru_read(4,(uint8_t *)&pru_data,4);
    printf("%d\n",pru_data);
    pru_read(5,(uint8_t *)&pru_data,4);
    printf("%d\n",pru_data);

    pru_channel_close(1);
    pru_channel_close(2);
    pru_channel_close(3);
    pru_channel_close(4);
    pru_channel_close(5);
    return 0;
}
