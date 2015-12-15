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

#include "libuart.h"

//#define UART_DEVICE                                                   "/dev/ttyUSB0"
#define UART_DEVICE							"/dev/console"
#define BLOCK_SIZE								64

extern void get_time(char *str_t);

int main(int argc, char **argv)
{
	int result = -1;
	fv_uart_t *uart_config;
	uart_config = malloc(sizeof(fv_uart_t));
	char *info = malloc(BLOCK_SIZE);
	char *dev = UART_DEVICE;
	char time_str[30];
	uint64_t passes = 1;
//      int rbaud = 115200;

	for (int i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (!strcmp(arg, "--dev") && i + 1 < argc) {
			dev = argv[++i];
		} else if (!strcmp(arg, "--passes") && i + 1 < argc) {
			passes = atoi(argv[++i]);
		} else {
			get_time(time_str);
			printf("[Console-tile-%s]: Unknow option: %s\n", time_str, arg);
			return -1;
		}
	}

	/** Init the device. */
	result = fv_uart_init(uart_config, dev);
	if (result == -1)
		printf("init error\n");

	/** Host send the packets first. */
	for (int i = 0; i < BLOCK_SIZE; i++) {
		info[i] = 'a' + i % 26;
	}

	char *recv_buf = malloc(BLOCK_SIZE);
	int *send_count = malloc(sizeof(int));
	int *recv_count = malloc(sizeof(int));
	for (uint64_t i = 0; i < passes; i++) {
		result = fv_uart_send(uart_config, info, BLOCK_SIZE, send_count);
		if (result == -1){
			printf("send error\n");
			return -1;
		}
                result = fv_uart_recv(uart_config, recv_buf, BLOCK_SIZE, recv_count);
                if (result == -1) {
                        printf("recv error\n");
		}
		for (int i = 0; i < BLOCK_SIZE; i++) {
			recv_buf[i] = 'a' + i % 26;
		}

		if (*recv_count != BLOCK_SIZE) {
			printf("recv failed!\n");
			return -1;
		}
		for (int j = 0; j < BLOCK_SIZE; j++) {
			if (((unsigned char *)recv_buf)[j] != 'a' + j % 26) {
				printf("%02x \n", recv_buf[j]);
				get_time(time_str);
				printf("[Console-tile-%s]: Check the received-%lu packets failed\n",
				     time_str, i + 1);
				return -1;
			}
		}
	}

	get_time(time_str);
	printf("[Console-tile-%s]: Test the bidirectional transmisson for %lu times success\n",
	     time_str, passes);
	free(info);
	info = NULL;
	free(recv_buf);
	recv_buf = NULL;
	free(recv_count);
	recv_count = NULL;
	free(send_count);
	send_count = NULL;

	return 0;
}
