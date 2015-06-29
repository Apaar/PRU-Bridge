//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
    while(check_init() != 1);

    uint8_t data[4] = "abcd";
    int data1 = 100;
    write_buffer(0,data,4);
    write_buffer(1,(uint8_t*)(&data1),sizeof(data1));
return 0;
}
