#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include "libi2c.h"

#define DEVICE 		"/dev/i2c-0"

extern void get_time(char *str_t);
extern uint64_t htoi(char s[]);  


int main(int argc, char* argv[])
{
    int result = -1;
    int passes = 1;
    int offset = 0;
    int slave_addr = 0;
    int interval = 1;
    int test_do = 1;
    int word_offset = 0;
    char time_str[30];
    int READ_BYTES = 10;
    char *dev = DEVICE;
    
    /** Parse the arguments. */
    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        if (!strcmp(arg, "--passes") && i + 1 < argc)
        {
            passes = atoi(argv[++i]);
        }
        else if (!strcmp(arg, "--interval") && i + 1 < argc)
        {
            interval = atoi(argv[++i]);
        }
        else if (!strcmp(arg, "--offset") && i + 1 < argc)
        {
            offset = htoi(argv[++i]);
        }
        else if (!strcmp(arg, "--i2c") && i + 1 < argc)
        {
            dev = argv[++i];
        }
        else if (!strcmp(arg, "--slave") && i + 1 < argc)
        {
            slave_addr = htoi(argv[++i]);
        }
        else if (!strcmp(arg, "--bytes") && i + 1 < argc)
        {
            READ_BYTES = atoi(argv[++i]);
        }
        else if (!strcmp(arg, "--word_offset"))
        {
            word_offset = SIXTEEN_BIT;
        }
        else if (!strcmp(arg, "--version"))
        {
            printf("[ROM-WRITE] version: V1.10\n");
        }
        else
        {
            printf("Unkown option: %s\n", arg);
            return -1;
        }
    }
    char* read_buffer = malloc(READ_BYTES);
    assert(read_buffer);
    char *bib_check = malloc(READ_BYTES);
    memset(read_buffer, 0, READ_BYTES);
    srand((unsigned int)time(NULL));
    for (int i = 0; i < READ_BYTES; i++)
    {
        bib_check[i] = random(100);
    }

    bib_check[0] = 0x5b;
    
    fv_i2c_t *i2c_config;
    i2c_config = malloc(sizeof(fv_i2c_t));
    result = fv_i2c_init(i2c_config, dev, slave_addr);
    if (result == -1)
	printf("init error!\n");
    int *write_count = malloc(sizeof(int));
    int *read_count = malloc(sizeof(int));
    while (test_do <= passes)
    {
	    result = fv_i2c_write(i2c_config, word_offset, offset, bib_check, READ_BYTES, write_count);
	    if (*write_count != READ_BYTES)
		printf("write error!\n");
	    result = fv_i2c_read(i2c_config, word_offset, offset, read_buffer, READ_BYTES, read_count);
            if (*read_count != READ_BYTES) 
	        printf("read error1012");

            else 
            {
#if 0
                for (int i = 0; i < READ_BYTES; i++)
                {
                    if(i && i % 8 == 0)
                    {
                        printf("\n");
                    }
                    printf("0x%02x ", ((unsigned char*)read_buffer)[i]);
                }
                printf("\n");
#endif
                for (int i = 0; i < READ_BYTES; i++)
                {
                    if (((unsigned char*)read_buffer)[i] != bib_check[i])
                    {
                        get_time(time_str);
                        printf("[BIB-READ-%s]: Check the data failed %d -- 0x%02x <---> 0x%02x\n", 
                                time_str, i, ((unsigned char*)read_buffer)[i], ((unsigned char *)bib_check)[i]);
                        return -1;                      
                    }
                }
            } 

            test_do++;   
            sleep(interval);  

    }

    get_time(time_str);
    printf("[BIB-READ-%s]: Read the data from 0x%04x[64] and check it success\n", 
            time_str, offset);
    free(read_buffer);
    read_buffer = NULL;
    free(i2c_config);
    i2c_config = NULL;
    free(bib_check); 
    bib_check = NULL; 
    free(write_count);
    write_count = NULL;
    free(read_count);
    read_count = NULL;  

    return 0;
}
