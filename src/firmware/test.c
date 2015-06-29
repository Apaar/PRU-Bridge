//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
    while(check_init() != 1);
    write_buffer(0,'a');
    write_buffer(1,'b');
    write_buffer(2,'c');
    write_buffer(3,'d');
return 0;
}
