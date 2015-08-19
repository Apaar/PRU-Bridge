# PRU-Bridge


A generic, multi channel bridge between userspace Linux and PRU allowing for easy and efficient data tranfer.

Each channel is unidirectional and will be represented by sysfs files in userspace.Number of channels has been fixed at 10 but length of each channel can be varied.


### Userspace Linux C API's(pru_bridge.h)
```javascript
int pru_bridge_init(int channel_sizes[NUM_CHANNELS]);   
//channel_sizes is a array of 10 int which contain channel sizes
void pru_channel_open(int channel_no,int type);         
//type is either PRU_READ or PRU_WRITE depending on channel type
void pru_channel_close(int channel_no);
int pru_write(int channel_no,uint8_t* pru_data,int length);
int pru_read(int channel_no,uint8_t* pru_data,int length);
```
### Userspace Linux Python API's
```javascript
init(channel_sizes)
//channel_sizes is a array of 10 int which contain channel sizes
channel_open(channel_no,channel_type)
channel_close(channel_no)
read(channel_no,data,length)
write(channel_no,data,length)
```

### PRU C API's
(not sure if all can be implemented due to code size constraints)
```javascript
int check_init()		//returns whether or not driver/channels have been initialisied until this is not '1' pru_bridge cannot be used
int read_buffer(int ring_no,uint8_t* pru_data,int length);
int write_buffer(int ring_no,uint8_t* pru_data,int length);
int check_index(int ring_no);
```

### How It Works

* #### ARM->PRU
    * Initialisation : a character array containing the size of channels, seperated by spaces is input to the sysfs file init.The handler first extracts the numbers from the buf being returned by the handler.Then the pru_bridge_init function computes the physical memory locations of each ring of each channel and buffers withing the channels and maps(ioremap) it to kernel memory.Finally the flag for driver initislisation is set to 1.
	* Write : write from pru to arm will take place via writing a character array to a channelno_write sysfs file.This inturn invokes the handler which runs a loop iterating through the char array and calls the pru_write function within the driver for every character.
Currently there are no checks on whether data is being overwritten.
	* Read :To read from a channel you read from the channelno_read sysfs fie.The handler then reads a single character and writes it to the sysfs file.Currently there is no checking on whether there is data in the channel or not,but adding it is a simple job as there is space in the control channel for the flags

* #### PRU->ARM
    * Init_check :This simply checks if the driver has initialsied the channels or not
	* Write : write currently is just a single character being directly written to the appropriate circular buffer without any checks 
	* Read :Read currently returns a single character without any checks on whether there is any data in the channel or not

I plan on getting the checks up in a bit i had disabled them as they were causing weird issues while testing the driver.

