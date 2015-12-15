#include "libi2c.h"
#define FAILURE -1
#define SUCCESS 0


void get_time(char *str_t);
uint64_t htoi(char s[]);  



void get_time(char *str_t)
{
    time_t now;
    char str[30];
    memset(str,0,sizeof(str));
    time(&now);
    strftime(str,30,"%Y-%m-%d %H:%M:%S",localtime(&now));
    int len = strlen(str);
    str[len] = '\0';
    strcpy(str_t,str);
}


uint64_t htoi(char s[])  
{  
    int i;  
    uint64_t n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X'))  
    {  
        i = 2;  
    }  
    else  
    {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)  
    {  
        if (tolower(s[i]) > '9')  
        {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  
        {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  

    return n;  
} 

int fv_i2c_init(fv_i2c_t *i2c_t, char * dev, int slave_addr)
{
    int fd;
    char time_str[30];
    int result;
    if (NULL == dev) {
	printf("dev is null!\n");
	return FAILURE;
    } 
	
    fd = open(dev, O_RDWR);
    if (fd < 0) {
	printf("open error!\n");
	return FAILURE;
    }
    if (NULL == i2c_t) {
	printf("i2c_t is NULL!\n");
	return FAILURE;
    }
    i2c_t->fd = fd;

    /** Select slave address 7-bit. */
    result = ioctl(fd, I2C_TENBIT, 0);
    if (result)
    {
        get_time(time_str);
        printf("[ROM-READ-%s]: Set the i2c address not 10 bits failed: %s\n",
                time_str, strerror(errno));
	return FAILURE;
    }

    /** 7-bit slave address 0x54. */
    result = ioctl(fd, I2C_SLAVE, slave_addr);
    if (result)
    {
        get_time(time_str);
        printf("[ROM-READ-%s]: Set the i2c address to 0x%02x failed: %s\n",
                time_str, slave_addr, strerror(errno));
	return FAILURE;
    }
    return SUCCESS;
}

int fv_i2c_write(fv_i2c_t *i2c_t, fv_sys_type_t word_offset, int offset, char *input_buffer, int write_bytes, int *write_count)
{
        if (NULL == i2c_t) {
	    printf("i2c_t is NULL!\n");
	    return FAILURE;
        }
        if (NULL == write_count) {
	    printf("write_count is NULL!\n");
	    return FAILURE;
        }
	char time_str[30];
	int result;
	char *write_buffer = malloc(write_bytes+2);
        if(word_offset == SIXTEEN_BIT)
        {
            /** We just write 64 bytes at the first of the rom. */
            write_buffer[0] = offset >> 8;
            write_buffer[1] = (offset & 0xFF); 

            memcpy((write_buffer + 2), input_buffer, write_bytes);

            result = write(i2c_t->fd, write_buffer, write_bytes + 2);
            if (result != (write_bytes+2))
            {
                get_time(time_str);
                printf("[ROM-READ-%s]: Write the data offset to 0x%04x failed: %s\n", 
                        time_str, offset, strerror(errno));
                printf("line 181 result %d \n",result);             
		return FAILURE;
            }
	    result = result-2;
	    *write_count = result;
	    free(write_buffer);
	    return SUCCESS;
		
        } else if(word_offset == EIGHT_BIT){
            /** We just write 64 bytes at the first of the rom. */
            write_buffer[0] = offset;

            memcpy((write_buffer + 1), input_buffer, write_bytes);

            result = write(i2c_t->fd, write_buffer, write_bytes + 1);
            if (result != write_bytes + 1)
            {
                get_time(time_str);
                printf("[ROM-READ-%s]: Write the data offset to 0x%04x failed: %s\n", 
                        time_str, offset, strerror(errno));              
		return FAILURE;
            }
	    result= result-1;
	    *write_count = result;
	    free(write_buffer);
	    return SUCCESS;
        }
	return FAILURE;
}



int fv_i2c_read(fv_i2c_t *i2c_t, fv_sys_type_t word_offset, int offset, char *output_buffer, int read_bytes, int *read_count)
{
        if (NULL == i2c_t) {
	    printf("i2c_t is NULL!\n");
	    return FAILURE;
        }
        if (NULL == read_count) {
	    printf("write_count is NULL!\n");
	    return FAILURE;
        }
	int result;
	char time_str[30];
	char write_buffer[2];
        if (word_offset == SIXTEEN_BIT)
        {
            write_buffer[0] = offset >> 8;
            write_buffer[1] = (offset & 0xFF);

            result = write(i2c_t->fd, write_buffer, 2);
            if (result != 2)
            {
                get_time(time_str);
                printf("[ROM-READ-%s]: Write the data offset to 0x%04x failed: %s\n",
                        time_str, offset, strerror(errno));
                return FAILURE;
            }
        }
        else if (word_offset == EIGHT_BIT)
        {
            write_buffer[0] = offset;

            result = write(i2c_t->fd, write_buffer, 1);
            if (result != 1)
            {
                get_time(time_str);
                printf("[ROM-READ-%s]: Write the data offset to 0x%04x failed: %s\n",
                        time_str, offset, strerror(errno));
                return FAILURE;
            }
        }

        /** Read the data at the Zero offset. */
        result = read(i2c_t->fd, output_buffer, read_bytes);
        if (result != read_bytes)
        {
	    printf("1");
            get_time(time_str);
            printf("[ROM-READ-%s]: Read the data from 0x%04x[64] failed: %s\n",
                    time_str, offset, strerror(errno));
            return FAILURE;
        }
	*read_count = result;
	return SUCCESS;
}
