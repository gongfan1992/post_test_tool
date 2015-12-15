# Default target.
all:

ifndef TILERA_ROOT
$(error The 'TILERA_ROOT' environment variable is not set.)
endif

CC = $(TILERA_ROOT)/bin/tile-cc
TILE_MONITOR = $(TILERA_ROOT)/bin/tile-monitor
PARALLELIZE = $(TILERA_ROOT)/bin/parallelize

OPT = -Os
CFLAGS = -std=gnu99 -Wall -g $(OPT)
LIBS = -lgxio 

SRCS = gpio_loop.c util.c 

OBJS = $(SRCS:%.c=%.o) 

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(XFLAGS) -c -o $@ $<    

gpio_loop: gpio_loop.o util.o
	$(CC) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS)

all: $(OBJS) gpio_loop

clean:
	rm -f *.o $(OBJS) gpio_loop

.PHONY: all clean

