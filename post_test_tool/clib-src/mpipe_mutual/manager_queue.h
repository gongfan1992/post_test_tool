/*************************************************************************
	> File Name: manager_queue.h
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 30 Apr 2014 03:30:41 PM CST
 ************************************************************************/
#ifndef manager_queue_MANAGER_H
#define manager_queue_MANAGER_H
typedef struct _manager_queue
{
    int push_pos;
    int pop_pos;
    void ** node;
    int item_size;
    unsigned int total;
    unsigned int length;
    pthread_mutex_t mutex;
    pthread_cond_t empty;
    pthread_cond_t full;
}manager_queue_t;
/*
* This is just a queue not a pool
* */
manager_queue_t * init_manager_queue(int length,int item_size);

void destroy_manager_queue(manager_queue_t * manager_queue);

bool push_session_buf(manager_queue_t * manager_queue,void * data);

bool pop_session_buf(manager_queue_t * manager_queue,void ** data);

bool is_empty_manager_queue(manager_queue_t * manager_queue);

bool is_full_manager_queue(manager_queue_t * manager_queue);
#endif
