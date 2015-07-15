
#!/usr/bin/env python

num_channels = 5
a = [None] * num_channels


def init(channel_sizes):
    char_size = ''
    for size in channel_sizes:
        char_size += str(size)
        char_size += ' '
    with open('/sys/devices/virtual/misc/pru_bridge/init', 'w') as f:
        f.write(char_size)

def channel_open(channel_no,channel_type):
    file_loc = '/sys/devices/virtual/misc/pru_bridge/ch'
    file_loc += str(channel_no)
    if channel_type == 0:
        file_loc += '_read'
    elif channel_type == 1:
        file_loc += '_write'
    a[channel_no - 1] = open(file_loc,'rw')
    print file_loc

def channel_close(channel_no):
    a[channel_no - 1].closed
    
def read(channel_no,data):
    data = a[channel_no - 1].read(1)

def write(channel_no,data):
    a[channel_no - 1].write(data)

sizes = [1000,0,0,0,0]
init(sizes)
channel_open(1,0)
data = 'a'
read(1,data)
print data
channel_close(1)

