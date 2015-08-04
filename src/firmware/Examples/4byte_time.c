//code to for lossless stream data transfer via pru_bridge


#include "pru_bridge.h"
#include "pru_defs.h"

static inline u32 read_PIEP_COUNT(void)
{
    return PIEP_COUNT;
}

int main()
{


    /* configure timer */
	PIEP_GLOBAL_CFG = GLOBAL_CFG_CNT_ENABLE	|
			  GLOBAL_CFG_DEFAULT_INC(1) |
			  GLOBAL_CFG_CMP_INC(1);
	PIEP_CMP_STATUS = CMD_STATUS_CMP_HIT(1);
	PIEP_CMP_CFG |= CMP_CFG_CMP_EN(1);

	int current = 0,last = 0,delay = 0;

    while(check_init() != 1);

    uint8_t check_flag = 0;         //getting the pru to wait until we are ready to read
    while(check_flag == 0)
    {
        read_buffer(1,&check_flag,1);
    }

    while(1)
    {
        //if(check_index(0))           //checking if the buffer is full or not
        //{
            current = read_PIEP_COUNT();
            delay = current - last;
            write_buffer(0,(uint8_t*)(&delay),4);
            last = current;
        //}
    }

return 0;
}
