//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
    while(check_init() != 1);
    int i = 10000;
    write_buffer(0,(uint8_t*)(&i),4);
return 0;
}
