

#include <stdint.h>
#include <stdio.h>

#include "pru_bridge.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "pru_bridge.h"

int main()
{
    int size[5] = {1000,0,0,0,0};
    pru_bridge_init(size);
    pru_channel_open(1,PRU_READ);
    int i=0;
    clock_t t1, t2;
    uint8_t pru_data;
    while(i<1000)
    {
        t1 = clock();
        pru_read(1,(uint8_t *)(&pru_data),1);
        printf("%d\n",pru_data);
        i++;
        t2 = clock();
        double diff = ((double)(t2 - t1)) / CLOCKS_PER_SEC;
        printf("%f\n",diff);
    }
    pru_channel_close(1);
    return 0;
}


