
#!/usr/bin/env python

sizes = [1000,0,0,0,0]
init(sizes)
channel_open(1,0)
data = 'a'
read(1,data)
print data
channel_close(1)
