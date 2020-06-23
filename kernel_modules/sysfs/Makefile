
CURRENT=$(shell uname -r)
KDIR=/lib/modules/$(CURRENT)/build
TARGET=hello2

obj-m += $(TARGET).o

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean