/*************************************************************************
	> File Name: manager_queue.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 30 Apr 2014 03:30:33 PM CST
 ************************************************************************/
#include "includes.h"
/*
* Date:     2014年4月30日15:53:00
* Author:   likeyi
* Function: 初始化一个所有空闲的池子。
* Input:
* Output:
* */
manager_queue_t * init_manager_queue(int numbers,int item_size)
{
    assert(numbers > 0);
    manager_queue_t * manager_queue = malloc(sizeof(manager_queue_t));
    exit_if_ptr_is_null(manager_queue,"alloc manager_queue error");
    manager_queue->node = malloc(numbers * sizeof(void *));
    exit_if_ptr_is_null(manager_queue->node,"alloc manager_queue node error");
    bzero(manager_queue->node,numbers);
    manager_queue->item_size = item_size;
    manager_queue->total = numbers;
    manager_queue->length = 0;
    manager_queue->pop_pos = 0;
    /*
    * 初始化时为满的。
    * */
    manager_queue->push_pos = 0;
    //pthread_mutex_init(&manager_queue->mutex,NULL);
    //pthread_cond_init(&manager_queue->empty,NULL);
    return manager_queue;
}
/*
* 销毁一个缓冲区池子
* 时间：2014年5月8日17:49:21
* 作者: likeyi
* */
void destroy_manager_queue(manager_queue_t * manager_queue)
{
    pthread_mutex_destroy(&manager_queue->mutex); 
    pthread_cond_destroy(&manager_queue->empty);
    free(manager_queue->node);
    manager_queue->node = NULL;
    free(manager_queue);
    manager_queue = NULL;
}
bool push_session_buf(manager_queue_t * manager_queue,void * data)
{
    //pthread_mutex_lock(&manager_queue->mutex);
    while(((manager_queue->push_pos + 1) % manager_queue->total == manager_queue->pop_pos))
    {
        //DEBUG("Error:manager_queue is full\n");
        //pthread_mutex_unlock(&manager_queue->mutex);
        //pthread_cond_wait(&manager_queue->full,&manager_queue->mutex);
		return false;
    }
    manager_queue->node[manager_queue->push_pos] = data;
    ++manager_queue->push_pos;
    ++manager_queue->length;
    if(manager_queue->push_pos >= manager_queue->total)
    {
        manager_queue->push_pos = 0;
    }
    //pthread_cond_signal(&manager_queue->empty);
    //pthread_mutex_unlock(&manager_queue->mutex);
    return true;
}
/*
*  本函数不用is_empty_manager_queue，因为防止锁中锁
* */
bool pop_session_buf(manager_queue_t * manager_queue,void ** data)
{
    //pthread_mutex_lock(&manager_queue->mutex);
    while(manager_queue->push_pos == manager_queue->pop_pos)
    {
        //DEBUG("Error:manager_queue is empty\n");
        //pthread_cond_wait(&manager_queue->empty,&manager_queue->mutex);
		return false;
    }
    *data = manager_queue->node[manager_queue->pop_pos];
    ++manager_queue->pop_pos;
    --manager_queue->length;
    if(manager_queue->pop_pos >= manager_queue->total)
    {
        manager_queue->pop_pos = 0;
    }
    //pthread_cond_signal(&manager_queue->full);
    //pthread_mutex_unlock(&manager_queue->mutex);
    return true;
}
/*
*  
* */
bool is_empty_manager_queue(manager_queue_t * manager_queue)
{
    bool result;
    //pthread_mutex_lock(&manager_queue->mutex);
    result = (manager_queue->push_pos == manager_queue->pop_pos);
    //pthread_mutex_unlock(&manager_queue->mutex);
    return result;
}
bool is_full_manager_queue(manager_queue_t * manager_queue)
{
    bool result;
    //pthread_mutex_lock(&manager_queue->mutex);
    result =((manager_queue->push_pos + 1) % manager_queue->total == manager_queue->pop_pos);
    //pthread_mutex_unlock(&manager_queue->mutex);
    return result;
}
