
/*
 * Userspace c library to utilise the pru_bridge driver
 *
 * Copyright (C) 2015 Apaar Gupta
 * This file is licensed under the terms of the GNU General Public License
 * version 3.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "pru_bridge.h"

#define NUM_CHANNELS 5
int file_name[NUM_CHANNELS];

int pru_bridge_init(int channel_sizes[NUM_CHANNELS])
{
    int i=0;
    char input[20],char_sizes[5];
    strcpy(input,"");
    for(i=0;i<NUM_CHANNELS;i++)
    {
        sprintf(char_sizes, "%d",channel_sizes[i]);
        strcat(input, char_sizes);
        strcat(input, " ");
    }
    strcat(input, "\n\0");
    int f = open("/sys/devices/virtual/misc/pru_bridge/init",O_WRONLY);
    if (f != -1)
    {
        write(f,input,sizeof(input));
        close(f);
    }
    else
    printf("Unable to open init\n");
}

void pru_channel_open(int channel_no,int type)
{
    char input[20];
    char ch_number[5];
    sprintf(ch_number,"%d",channel_no);
    strcpy(input,"");
    strcat(input,"ch");
    strcat(input,ch_number);

    if(type == 0)
        strcat(input, "_read");
    else if(type == 1)
        strcat(input, "_write");

    char file_nm[100] = "/sys/devices/virtual/misc/pru_bridge/";
    strcat(file_nm, input);
    printf("%s\n",file_nm);
    file_name[channel_no-1] = open(file_nm,O_RDWR);
}

void pru_channel_close(int channel_no)
{
   close(file_name[channel_no-1]);
}

int pru_write(int channel_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        printf("%c\n",*(pru_data+i));
        write(file_name[channel_no-1],(pru_data+i),1);
	i++;
    }
return length;
}

int pru_read(int channel_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        read(file_name[channel_no-1],(pru_data+i),1);
        printf("%d\n",*(pru_data+i));
        i++;
    }
return length;
}
