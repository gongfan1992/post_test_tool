# Default target.
all:

ifndef TILERA_ROOT
$(error The 'TILERA_ROOT' environment variable is not set.)
endif

CC = $(TILERA_ROOT)/bin/tile-cc
TILE_MONITOR = $(TILERA_ROOT)/bin/tile-monitor
PARALLELIZE = $(TILERA_ROOT)/bin/parallelize
AR = ar

OPT = -Os
CFLAGS = -std=gnu99 -Wall -Werror -g $(OPT)
LIBS = -lgxio 

SRCS = rom_write.c libi2c.c

OBJS = $(SRCS:%.c=%.o) 

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(XFLAGS) -c -o $@ $<    

libi2c.a: libi2c.o
	rm -f $@
	$(AR) cq $@ $^

rom_write: rom_write.o libi2c.o
	$(CC) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS)

all: $(OBJS) rom_write libi2c.a

clean:
	rm -f *.o $(OBJS) rom_write lib*.a

.PHONY: all clean

