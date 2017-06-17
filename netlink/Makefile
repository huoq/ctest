MODULE_NAME :=netlink_kernel
obj-m :=$(MODULE_NAME).o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	gcc -o netlink_userspace netlink_userspace.c
clean:
	rm -fr *.ko *.o *.cmd netlink_userspcae $(MODULE_NAME).mod.c
