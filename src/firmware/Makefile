A_DIR := /usr/share/ti/cgt-pru/lib

all: firmware

firmware: pru_bridge.obj test.obj AM3359_PRU.cmd
	lnkpru -i$(A_DIR) pru_bridge.obj test.obj AM3359_PRU.cmd -o=firmware

pru_bridge.obj: pru_bridge.c pru_bridge.h pru_defs.h
	clpru -O3 --c99 pru_bridge.c 

test.obj: test.c pru_bridge.h
	clpru -O3 --c99 test.c

install: all
	install -m 444 firmware /lib/firmware

clean:
	rm firmware *.obj
