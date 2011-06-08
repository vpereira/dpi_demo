LIB_ROOT = $(PWD)

LIB_DIRS = lib/dataplane libpcap src #pkt_gen

all: make_pkg

make_pkg:
	@echo ""
	@for dirs in $(LIB_DIRS); do \
		if [ -d $${dirs} ]; then \
			echo "******************   Entering [$${dirs}]"; 	\
			cd $${dirs};		\
			$(MAKE);			\
			if [ $$? != '0' ]; then 			\
				echo "";						\
				exit 1;							\
			fi;									\
			echo "******************   Leaving [$${dirs}]"; 	\
			echo "";							\
			cd $(LIB_ROOT); \
			else		\
				echo "Can't find directory [$${dirs}]";	\
				exit 1;									\
		fi;				\
	done;

clean:
	@echo ""
	@for dirs in $(LIB_DIRS); do \
		if [ -d $${dirs} ]; then \
			echo "******************   Entering [$${dirs}]"; \
			cd $${dirs}; \
			$(MAKE) clean; \
			echo "******************   Leaving [$${dirs}]"; \
			echo "";	\
			cd $(LIB_ROOT); \
			else		\
				echo "Can't find directory [$${dirs}]";	\
				exit 1;									\
		fi;	\
	done;


