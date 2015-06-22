//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
while(check_init() != 1);
pru_bridge_init();
write_buffer(0,100);
write_buffer(1,101);
write_buffer(2,102);
write_buffer(3,808);
write_buffer(4,666);

return 0;
}
