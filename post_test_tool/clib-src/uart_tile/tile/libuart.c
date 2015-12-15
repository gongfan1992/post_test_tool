#include "libuart.h"

#define FAILURE -1
#define SUCCESS 0
#define MIN_SIZE  64

void get_time(char *str_t);

void get_time(char *str_t)
{
	time_t now;
	char str[30];
	memset(str, 0, sizeof(str));
	time(&now);
	strftime(str, 30, "%Y-%m-%d %H:%M:%S", localtime(&now));
	int len = strlen(str);
	str[len] = '\0';
	strcpy(str_t, str);
}

int fv_uart_init(fv_uart_t * uart_t, char *device)
{
	if (uart_t == NULL) {
		printf("uart_t is NULL");
		return FAILURE;
	}
	int fd;
	char time_str[30];
	int result;
	struct termios ts;
	/** Open the device. */
	fd = open(device, O_RDWR | O_NONBLOCK);
	uart_t->fd = fd;
	if (fd < 0) {
		get_time(time_str);
		printf("[Console-%s]: Open the device %s failed: %s\n",
		       time_str, device, strerror(errno));
		return FAILURE;
	}

	/** Set the speed to 115200. */
	result = tcgetattr(fd, &ts);
	if (result) {
		get_time(time_str);
		printf("[Console-%s]: Get the attribute of device %s failed: %s\n", time_str,
		       device, strerror(errno));
		return FAILURE;
	}

	tcflush(fd, TCIOFLUSH);

	result = cfsetispeed(&ts, B115200);
	if (result) {
		get_time(time_str);
		printf("[Console-%s]: Set the input Baul rate %d failed: %s\n",
		     time_str, 115200, strerror(errno));
		return FAILURE;
	}

	result = cfsetospeed(&ts, B115200);
	if (result) {
		get_time(time_str);
		printf("[Console-%s]: Set the output Baul rate %d failed: %s\n",
		     time_str, 115200, strerror(errno));
		return FAILURE;
	}

	tcsetattr(fd, TCSANOW, &ts);

	tcflush(fd, TCIOFLUSH);

	/** Set parity of the uart. */
	result = tcgetattr(fd, &ts);
	assert(!result);

	ts.c_cflag &= ~(CSIZE | PARENB | CRTSCTS | CSTOP);
	ts.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | ICRNL | INPCK);
	ts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | NOFLSH);
	ts.c_oflag &= ~(OPOST | ONLCR | OCRNL);

	ts.c_cflag |= (CREAD | CS8);
	tcflush(fd, TCIFLUSH);
	ts.c_cc[VTIME] = 0;
	ts.c_cc[VMIN] = MIN_SIZE;
	tcsetattr(fd, TCSANOW, &ts);

	get_time(time_str);
	printf("[Console-%s]: Host is ready to test the rbaud: %d\n", time_str, 115200);
	return SUCCESS;
}

int fv_uart_send(fv_uart_t * uart_t, char *send_buf, int send_bytes, int *send_count)
{
	if (uart_t == NULL) {
		printf("uart_t is NULL");
		return FAILURE;
	}
	if (send_count == NULL) {
		printf("send_count is NULL");
		return FAILURE;
	}
	int result;
	char time_str[30];
	result = write(uart_t->fd, send_buf, send_bytes);
	if (result != send_bytes) {
		get_time(time_str);
		printf("[Console-%s]: Host send the packets failed: %s\n",
		       time_str, strerror(errno));
		return FAILURE;
	}
	*send_count = result;
	return SUCCESS;
}

int fv_uart_recv(fv_uart_t * uart_t, char *recv_buf, int recv_bytes, int *recv_count)
{
	if (uart_t == NULL) {
		printf("uart_t is NULL");
		return FAILURE;
	}
	if (recv_count == NULL) {
		printf("recv_count is NULL");
		return FAILURE;
	}
	int result;
	char time_str[30];
	fd_set readfds;
	struct timeval timeout;

	FD_ZERO(&readfds);
	FD_SET(uart_t->fd, &readfds);
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	result = select(uart_t->fd + 1, &readfds, NULL, NULL, &timeout);
	if (result == -1) {
		get_time(time_str);
		printf("[Console-%s]: Host monitor the read file error: %s\n",
		     time_str, strerror(errno));
		return FAILURE;
	} else if (result == 0) {
		get_time(time_str);
		printf("[Console-%s]: Host read the packets timeout\n", time_str);
		return FAILURE;
	} else if (result == 1) {
		result = read(uart_t->fd, recv_buf, recv_bytes);
		if (result != recv_bytes) {
			printf("read count error!\n");
			return FAILURE;
		}
		*recv_count = result;
#if 0
		printf("============ Dump the received information[host] ============\n");
		for (int j = 0; j < recv_bytes; j++) {
			printf("%02x ", ((unsigned char *)recv_buf)[j]);
		}
		printf("\n");
#endif
	}

	return SUCCESS;
}
