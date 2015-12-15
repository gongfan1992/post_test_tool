#ifndef _LIBI2C_H
#define _LIBI2C_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>  
#include <ctype.h>      
#include <sys/types.h>  
#include <sys/stat.h>           
#include <sys/ioctl.h>  
#include <gxio/gpio.h>  
#include <fcntl.h>  
#include <time.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "libi2c.h"
#define random(x) (rand()%(x))

typedef struct fv_i2c{
	int fd;
}fv_i2c_t;


typedef enum {
	EIGHT_BIT,
	SIXTEEN_BIT,
}fv_sys_type_t;

int fv_i2c_init(fv_i2c_t *i2c_t, char *dev, int slave_addr);
int fv_i2c_read(fv_i2c_t *i2c_t, fv_sys_type_t word_offset, int offset, char *output_buffer, int read_bytes, int *read_count);
int fv_i2c_write(fv_i2c_t *i2c_t, fv_sys_type_t word_offset, int offset, char *input_buffer, int write_bytes, int *write_count);

#endif
