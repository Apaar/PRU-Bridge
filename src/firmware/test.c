//code to test pru_bridge
#include "pru_bridge.h"

int main()
{
while(check_init() != 1);
pru_bridge_init();
write_buffer(0,'p');
write_buffer(1,'p');
write_buffer(2,'p');
write_buffer(3,'p');
write_buffer(4,'p');

return 0;
}
