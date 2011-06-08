KERNELDIR := /home/kwan/project/SPG/OCTEON-SDK/linux/kernel_2.6/linux
CROSS_COMPILE = mips64-octeon-linux-gnu-

CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
LD      = $(CROSS_COMPILE)ld
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy

PCAP_ROOT = ../libpcap
PCAP_PATH = ${PCAP_ROOT}
PCAP_INCLUDE = ${PCAP_ROOT}

LIBS = ${PCAP_PATH}/libpcap.a

CFLAGS = -I. -I../include -I../lib/dataplane -I$(PCAP_INCLUDE) -DNLM_HW_FMS -DNO_COMPILER



default:
	gcc -I. -I../include fms_compiler.c -o fms_compiler -lfms_compiler -L../lib/
	$(CC) -I. -I../include -DNO_COMPILER -DNLM_HW_FMS demo.c -o demo -lpacket_api_fms -L../lib/
	$(CC) -I. -I../include -I../lib/dataplane -DNO_COMPILER -DNLM_HW_FMS -D_GNU_SOURCE mtdemo.c -o demo_mt -lpacket_api_fms -L../lib/ -lpthread
	$(CC) $(CFLAGS) dpi_demo.c -o dpi_demo $(LIBS) -lpacket_api_fms -L../lib/ -lpthread
#	$(CC) -I. -I../include -I$(PCAP_INCLUDE) -DNO_COMPILER -DNLM_HW_FMS dpi_demo.c -o dpi_demo $(LIBS) -lpacket_api_fms -L../lib/
	cp driver/netl7driver.ko demo dpi_demo ../pkg

clean:
	rm -f rxc demo demo_mt demo_ring
