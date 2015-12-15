/*************************************************************************
	> File Name: packet_generator.h
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 30 Apr 2014 03:02:21 PM CST
 ************************************************************************/
#ifndef PACKET_GENERATOR_H
#define PACKET_GENERATOR_H
typedef struct generator
{
    pthread_t   id;
    int         index;
    int         next_thread_id;
    uint64_t    total_send_byte;
    uint64_t    drop_total;
    config_t  * config;
    pool_t    * pool;
    int         rank;
    mpipe_common_t * mpipe_config;
	manager_queue_t * queue;
	uint64_t error_len;
	uint64_t error_data;
	uint64_t correct_len;
	uint64_t correct_data;
	uint64_t error_recv;
	uint64_t pool_empty;
	uint64_t pool_full;
	uint64_t queue_empty;
	uint64_t queue_full;
}generator_t;
typedef struct generator_set
{
    generator_t * generator;
	char * link_name;
    uint32_t numbers;
    config_t  * config;
}generator_set_t;
extern generator_set_t * generator_set;
extern config_t * config;
void   init_generator_set(config_t * config);
/*
 * 销毁线程，和销毁线程的数据结构要分两步走。
 * 尤其是当这个线程和其他线程有交互时。
 * */
void   destroy_generator(generator_set_t * generator_set);
void   finish_generator(generator_set_t * generator_set);
void * packet_generator_loop(void * arg);
void   print_counter_info(int sig);
#endif
