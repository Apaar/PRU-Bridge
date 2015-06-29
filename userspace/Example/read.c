#include "pru_bridge.h"


int main()
{
    int size[5] = {12,12,12,12,12};
    pru_bridge_init(size);
    pru_channel_open(1,PRU_READ);
    int pru_data[2];

    pru_read(1,pru_data,1);
    printf("%d\n",*pru_data);
    pru_channel_close(1);
    return 0;
}
