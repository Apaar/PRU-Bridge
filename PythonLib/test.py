
#!/usr/bin/env python

import pru_bridge

#sizes = [1000,0,0,0,0]
#init(sizes)
pru_bridge.channel_open(1,0)
data = 'a'
pru_bridge.read(1,data,1)
print data
pru_bridge.channel_close(1)
