#!/bin/sh
ifconfig eth6_0 192.168.55.108
arp -s 192.168.55.104 f8:1e:df:86:ee:2f 

insmod  netl7driver.ko
#./xaui_demo -d simple_rules_data.bin -a 9 
./demo_ring -d simple_rules_data.bin -a 9 -m 1 -v

#./pkg_gen -p -I eth1 -d statless 192.168.56.107 192.168.56.104
#./pkg_gen -p -I eth1 -d init 192.168.56.107 192.168.56.104
#./pkg_gen -p -I eth1 192.168.56.107 192.168.56.104

