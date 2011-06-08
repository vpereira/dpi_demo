#!/bin/bash

cd dataplane
gcc -O2 -c -DNLM_HW_FMS -I../../include -I../../src *.c
cd -
ar r libpacket_api_fms.a dataplane/*.o
