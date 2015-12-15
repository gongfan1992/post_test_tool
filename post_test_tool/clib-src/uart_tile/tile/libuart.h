#ifndef _LIBUART_H
#define _LIBUART_H

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct fv_uart{
	int fd;
}fv_uart_t;


int fv_uart_init(fv_uart_t *uart_t, char *device);
int fv_uart_send(fv_uart_t *uart_t, char *send_buf, int send_bytes, int *send_count);
int fv_uart_recv(fv_uart_t *uart_t, char *recv_buf, int recv_bytes, int *recv_count);

#endif
