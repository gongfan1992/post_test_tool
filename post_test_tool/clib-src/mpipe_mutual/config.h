/**
=========================================================================
 Author: findstr
 Email: findstr@sina.com
 File Name: config.h
 Description: (C)  2014-03  findstr
 Edit History: 
   2014-03-06    File created.
=========================================================================
**/
#ifndef CONFIG_H_H
#define CONFIG_H_H
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <dirent.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#define PROCBLOCK_SIZE 32
#define PAGE_SHIFT     12
#define CONFIG_FILE "./generator.conf"
#define PACKET_POOL_SIZE 2048
#define NODE_POOL_SIZE 1000000
#define PACKET_PARSER_NUMS 1
typedef struct __config
{
    int protocol;
    unsigned char srcmac[6];
    unsigned char dstmac[6];
    uint32_t saddr_min;
    uint32_t daddr_min;
    uint32_t saddr_cur;
    uint32_t daddr_cur;
    uint32_t saddr_max;
    uint32_t daddr_max;
    uint32_t sport_min;
    uint32_t dport_min;
    uint32_t sport_cur;
    uint32_t dport_cur;
    uint32_t sport_max;
    uint32_t dport_max;
    uint32_t speed;
    char * pkt_data;
    int packet_pool_size;   /* PACKET pool 的大小*/
    int pktlen;
    uint32_t period;
    char * send_link_name;
	char * recv_link_name;
    size_t  notif_ring_entries;
    uint32_t equeue_entries;
    int     per_worker_buckets;
    int     num_workers;
	int     once_packet_nums;
    gxio_mpipe_rules_dmac_t dmac;
    uint64_t timeout;
    uint32_t check_level;
    uint64_t errornum;
}config_t;

int ether_atoe(const char *a, unsigned char *e);

char * ether_etoa(const unsigned char *e, char *a);

int read_config_file(const char * file_name, config_t * config);

void print_config_file(config_t * config);

void exit_if_ptr_is_null(void * p,const char * c);
#endif
