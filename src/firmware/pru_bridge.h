
/*
 * Header file of PRU Bridge firmware to enable data trsfer between PRU<-->ARM
 *
 * Copyright (C) 2015 Apaar Gupta
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#ifndef PRU_BRIDGE_H
#define PRU_BRIDGE_H

#include <stdint.h>

int check_init(void);
int read_buffer(int ring_no,uint8_t* pru_data,int length);
int write_buffer(int ring_no,uint8_t* pru_data,int length);
int check_index(int ring_no);

#endif
