# Default target.
all:

ifndef TILERA_ROOT
$(error The 'TILERA_ROOT' environment variable is not set.)
endif

CC = $(TILERA_ROOT)/bin/tile-cc

OPT = -Os
CFLAGS = -std=gnu99 -Wall -Werror -g $(OPT) 
LDFLAGS = -static   
LIBS = -ltmc -lgxio -lpthread 

SRCS = mpsock.c util.c mutual_aging.c 

OBJS = $(SRCS:%.c=%.o)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(XFLAGS) -c -o $@ $<
 
mutual_aging: mpsock.o mutual_aging.o util.o
	$(CC) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS)

all: mutual_aging

clean:
	rm -f *.o *.d mutual_aging

.PHONY: all clean
