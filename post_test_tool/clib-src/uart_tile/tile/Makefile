# Default target.
all:

TILE_CC = tile-gcc
HOST_CC = gcc
AR = ar

OPT = -Os
CFLAGS = -std=gnu99 -Wall -Werror -g $(OPT)
LDFLAGS = $(if $(filter -m32, $(XFLAGS)), -static)
LIBS_TILE = 

HOST_OPT = -o2
HOST_CFLAGS = -std=gnu99 -Wall -Werror -g $(HOST_OPT) \
			  -I$(TILERA_ROOT)/include -DTILEPCI_HOST
LIBS_HOST = -lpthread

SRCS = uart_tile.c libuart.c

OBJS = $(SRCS:%.c=%.o) 

$(OBJS): %.o : %.c
	$(TILE_CC) $(CFLAGS) $(LDFLAGS) $(XFLAGS) -c -o $@ $< $(LIBS_TILE)   

libuart.a: libuart.o
	rm -f $@
	$(AR) cq $@ $^

uart_tile: uart_tile.o libuart.o
	$(TILE_CC) $(CFLAGS) $(LDFLAGS) $(XFLAGS) -o $@ $^ $(LIBS_TILE)   

all: $(OBJS) uart_tile libuart.a

clean:
	rm -f *.o uart_tile lib*.a

.PHONY: all clean
