//code to for lossless stream data transfer via pru_bridge


#include "pru_bridge.h"

int main()
{
    while(check_init() != 1);

    uint8_t check_flag = 0;         //getting the pru to wait until we are ready to read
    while(check_flag == 0)
    {
        read_buffer(1,&check_flag,1);
    }

    uint8_t i = 0;
    while(1)
    {
        if(check_index(0))           //checking if the buffer is full or not
        {
            write_buffer(0,(uint8_t*)(&i),1);
            i++;
        }
    }

return 0;
}
