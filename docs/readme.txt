This package contains SDK for NLS2008 family of NETL7 devices.

********************************
Change log w.r.t. release 2.6.1
********************************

- Optimized peak memory usage for large databases.


****************************
Package Directory Structure
****************************

docs/netl7_compiler_api.pdf  -- compiler/database API manual
docs/netl7_dataplane_api.pdf -- dataplane API manual
docs/netl7_rulefile_spec.pdf -- description of rule file format for fms_compiler.c
docs/netl7_devdrv_instr.txt  -- device driver instructions

include/nlm_packet_api.h     -- dataplane API
include/nlm_database_api.h   -- compiler/database API

lib/                         -- compiler and dataplane libraries/sources(optional)

src/demo.c                   -- dataplane demo
src/mtdemo.c                 -- dataplane multi_thread demo
src/ringdemo.c               -- dataplane multi_ring demo
src/fms_compiler.c           -- compiler demo
src/driver/                  -- demo driver

Recommended environment:

linux kernel 2.6.21+
gcc 4.1+
glibc 2.6+

To build and install demo driver, read docs/netl7_devdrv_instr.txt
and follow:

$ cd src/driver
$ make

For simple demo run use:
$ sudo insmod ./netl7driver.ko

For advanced evaluation (more than one device, multiple flows, multiple rings,
customized database, external traffic, etc) demo driver and kerneldemo should
be loaded as:
$ sudo insmod netl7driver.ko fixed_phys_base=XXXXX fixed_phys_size=YYYYY verbose=1

Do NOT use demo driver for production. Write OS specific nlm_prepare_device_config()
or set up fields of struct nlm_device_config manually in the way appropriate
for environment. See docs/netl7_dataplane_api.pdf for details.

If package contains dataplane sources, the steps to build them are:

1. cd lib
2. build_fms_dataplane.sh  # builds dataplane library
3. cd -

then build compiler and dataplane demos:

4. cd src
5. make           # builds compiler/demos

and run demo, demo_mt, demo_ring

$ ./demo
$ ./demo_mt
$ ./demo_ring

To compile <rule_file> into binary image <output_file> use:

$ fms_compiler <rule_file> -o <output_file>

fms_compiler.c expects rules to be specified in the format described in netl7_rulefile_spec.pdf

Sample input file:
# comment
[1]
1 a.*b
2 c.*d.*e
3 ^.{0,100}huh

[2:1000:group_2]
2 1000 pattern_2 a.*b
min_match_length 12
3 1000 pattern_3 abcd
4 1000 pattern_4 abcde
priority 2
# non-greedy a.*b is the following:
5 1100 pattern_5 a.*b
# or the following:
6 1000 xxx a.*b
non_greedy

[3]
10 a.*b
min_match_length 100
11 a.*b
non_greedy
12 a.*b
13 a.*b
match_once

