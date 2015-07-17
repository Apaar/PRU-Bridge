
/*
 * Userspace c library to utilise the pru_bridge driver
 *
 * Copyright (C) 2015 Apaar Gupta
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */


#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "pru_bridge.h"

#define NUM_CHANNELS 5
FILE *file_name[NUM_CHANNELS];

int pru_bridge_init(int channel_sizes[NUM_CHANNELS])
{
    int i=0;
    char input[20],char_sizes[5];
    strcpy(input,"");

    int size_check = 0;

    for(i=0;i<NUM_CHANNELS;i++)
    {
        sprintf(char_sizes, "%d",channel_sizes[i]);
        strcat(input, char_sizes);
        strcat(input, " ");
        size_check += channel_sizes[i];
        if(channel_sizes[i] < 0)
        {
            printf("Invalid Size\n");
            return 1;
        }
    }
    if(size_check > 11500)          //500 bytes should be enough to accomodate control channel
    {
        printf("Buffer size insufficient\n");
        return 1;
    }

    strcat(input, "\n\0");
    FILE* fp = fopen("/sys/devices/virtual/misc/pru_bridge/init","w");
    printf("Init opened\n");
    if (fp != NULL)
    {
        fwrite(input,sizeof(char),strlen(input),fp);
        fclose(fp);
    }
    else
        printf("Unable to open init\n");

    return 0;
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
    file_name[channel_no-1] = fopen(file_nm,"r+");
    if (file_name[channel_no-1] == NULL)
    {
        printf("Unable to open init\n");
    }

}

void pru_channel_close(int channel_no)
{
   fclose(file_name[channel_no-1]);
}

int pru_write(int channel_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        printf("%d\n",*(pru_data+i));
        fwrite((pru_data+i),1,1,file_name[channel_no-1]);
        rewind(file_name[channel_no-1]);
        i++;
    }
return length;
}

int pru_read(int channel_no,uint8_t* pru_data,uint8_t length)
{
    int i = 0;
    while(i<length)
    {
        fread((pru_data+i),1,1,file_name[channel_no-1]);
        printf("%d\n",*(pru_data+i));
        rewind(file_name[channel_no-1]);
        i++;
    }
return length;
}
