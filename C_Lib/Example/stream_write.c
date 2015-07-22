

#include <stdint.h>
#include <stdio.h>

#include "pru_bridge.h"

int main()
{
    int size[5] = {1000,0,0,0,0};
    pru_bridge_init(size);
    pru_channel_open(1,PRU_WRITE);
    int i=0;
    while(i<1000)
    {
        pru_write(1,(uint8_t *)(&i),4);
        i++;
    }
    pru_channel_close(1);
    return 0;
}




