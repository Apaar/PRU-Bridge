#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define NUM_CHANNELS 5
#define PRU_READ 0
#define PRU_WRITE 1

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
    file_name[channel_no] = open(file_nm,O_RDWR);
}

void pru_channel_close(int channel_no)
{
   close(file_name[channel_no]);
}

void pru_write(int channel_no,int data)
{
    char char_data[5];
    sprintf(char_data,"%d",data);
    printf("%s\n",char_data);
    write(file_name[channel_no],&char_data,sizeof(char_data));
}
//void pru_block_write(int channel_no,int* data,int length)

int pru_read(int channel_no)
{
    int data;
    read(file_name[channel_no],&data,sizeof(int));
    return data;
}

//int* pru_block_read(int channel_no,int length)

int main()
{
    int size[5] = {12,12,12,12,12};
    pru_bridge_init(size);
    pru_channel_open(1,PRU_WRITE);
    pru_channel_open(2,PRU_READ);
    pru_write(1,101);
    printf("%d\n",pru_read(2));
    pru_channel_close(1);
    pru_channel_close(2);
   return 0;
}
