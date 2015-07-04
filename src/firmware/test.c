//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
    while(check_init() != 1);
    int i =0;

    while(i<1000)
    {
        int data = i;
        write_buffer(0,(uint8_t*)(&data),sizeof(data));
        i++;
    }
return 0;
}
