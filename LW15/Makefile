
CURRENT=$(shell uname -r)
KDIR=/lib/modules/$(CURRENT)/build
PWD=$(shell pwd)
DEST=/lib/modules/$(CURRENT)/misc

TARGET1=LW15_1
TARGET2=kb_led
TARGET3=netlink_km

obj-m := $(TARGET1).o $(TARGET2).o $(TARGET3).o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
	@rm -rf .*.mk *.ko .*.o.d	
