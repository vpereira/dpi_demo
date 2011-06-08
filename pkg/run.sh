#!/bin/sh
ifconfig vifd 192.168.56.108
./pkg_gen -p -I eth1 -d statless 192.168.56.107 192.168.56.104
#./pkg_gen -p -I eth1 -d init 192.168.56.107 192.168.56.104
#./pkg_gen -p -I eth1 192.168.56.107 192.168.56.104

