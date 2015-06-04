KSRC = /lib/modules/$(shell uname -r)/build

obj-m := pru_bridge.o

all:
	@make -C $(KSRC) M=$(PWD) modules

clean:
	@make -C $(KSRC) M=$(PWD) clean
