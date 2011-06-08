KERNELDIR := /home/kwan/project/SPG/OCTEON-SDK/linux/kernel_2.6/linux
CROSS_COMPILE = mips64-octeon-linux-gnu-

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy

TARGET = libpacket_api_fms.a

CFLAGS = -O2  #-D__mips -D__GNUC__

all:$(TARGET)
$(TARGET):$(OBJS)
	$(AR) r $@ ./*.o
	cp $@ ../

%.o : %.c
	@echo compiling... $<
	$(CC) $(CFLAGS) -c -DNLM_HW_FMS -I../../include -I../../src *.c
#	$(CC) $(CFLAGS) -c  $<

clean:
	rm -f *.o *.a
