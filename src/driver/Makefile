obj-m := netl7driver.o

#KERNELDIR := /lib/modules/$(shell uname -r)/build
#KERNELDIR := /home/kwan/project/SPG/OCTEON-SDK/linux/kernel_2.6/linux
KERNELDIR := /home/kwan/project/SPG/linux-2.6-secondary
CROSS_COMPILE = mips64-octeon-linux-gnu-

CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy

## useful option for building modules:
##     make KBUILD_VERBOSE=1
##     make V=1

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o *.ko *.mod.c *.mod.o .*.cmd .tmp_versions Module.symvers *~
