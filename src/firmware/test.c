//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
    int err = 0;
    while(check_init() != 1);
        pru_bridge_init();
    int* data;
    int d = 101;
    data = &d;
    err = write_buffer(0,data,4);
    //err = write_buffer(1,data,4);
    //err = write_buffer(2,data,4);
    //err = write_buffer(3,data,4);
    //err = write_buffer(4,data,4);

return 0;
}
