#Default Target.

ifeq ($(filter tile%,$(shell uname -m)),)


ifdef TILERA_ROOT
CC 			= $(TILERA_ROOT)/bin/tile-gcc
MPIPE_CC 	= $(TILERA_ROOT)/bin/tile-mpipe-cc
LDFLAGS		= -lpthread -ltmc -lgxio

MONITOR  	= ${TILERA_ROOT}/bin/tile-monitor   

BOOT_ARGS   := --classifier ./classifier \
               --hvx crashinfo \
               --hvd POST=quick \
               --hvx dataplane=1-35 \
               --hvx hugepagesz=64m  --hvx hugepages=$(HUGEPAGES) \
               --hvx TLR_NETWORK=none \
               --vmlinux ./file_412/vmlinux \
               --upload-tile-libs selinux

MONITOR_BASIC_ARGS  = --debug-on-crash --upload ./ /udp \
					  --run -+- cd /udp  -+- \
                      --run -+- ./udp -+- \


else
$(error The 'TILERA_ROOT' enviroment variable is not set.)
endif

else
#local tilera platform
CC = gcc
MPIPE_CC=mpipe-cc
LDFLAGS= -lpthread -ltmc -lgxio
endif

OPT = -Os
CFLAGS= -std=gnu99 -Wall -O3
OBJS += config.o packet_generator.o pool_manager.o \
		checksum.o mpipe.o parse.o manager_queue.o 



.PHONY:clean all


EXECS=mpipe_mutual 
	 
all:$(EXECS)


mpipe_mutual:mpipe_mutual.c $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run_dev:  all 
	$(MONITOR) --dev gxpci0 --hv-bin-dir ./file_412/hv $(BOOT_ARGS) $(MONITOR_BASIC_ARGS) 

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f *.o $(EXECS)

