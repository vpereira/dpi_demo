#!/bin/sh
ifconfig eth6_0 192.168.55.100
arp -s 192.168.55.104 f8:1e:df:86:ee:2f
arp -s 192.168.55.106 f8:1e:df:86:ee:2e

insmod  netl7driver.ko
insmod dpi_drv.ko
ifconfig vifd 192.168.57.100

./xaui_demo  -d simple_rules_data.bin  -a 9

