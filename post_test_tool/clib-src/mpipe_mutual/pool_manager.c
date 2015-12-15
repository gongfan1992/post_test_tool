/*************************************************************************
	> File Name: pool_manager.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 30 Apr 2014 03:30:33 PM CST
 ************************************************************************/
#include "includes.h"
//pool_t * packet_pool, * session_pool, * buffer_pool;
//pool_t * manager_node_pool;
/*
* Date:     2014年4月30日15:53:00
* Author:   likeyi
* Function: 初始化一个所有空闲的池子。
* Input:
* Output:
* */
pool_t * init_pool(pool_type_t type,int numbers,int item_size)
{
    assert(numbers > 0);
    pool_t * pool = malloc(sizeof(pool_t));
    exit_if_ptr_is_null(pool,"alloc pool error");
    pool->node = malloc(numbers * sizeof(void *));
    exit_if_ptr_is_null(pool->node,"alloc pool node error");
    pool->buffer = malloc(numbers * item_size);
    exit_if_ptr_is_null(pool->buffer,"alloc pool buffer error");
    int i = 0;
    for(i = 0; i < numbers; ++i)
    {
        /*
        * 这个复杂的复制时为了，让node_t[]数组里面的指针指向真实的buffer.
        * */
        pool->node[i] = pool->buffer + i * item_size;
    }
    pool->item_size = item_size;
    pool->total = numbers;
    pool->free_num = numbers;
    pool->pop_pos = 0;
    /*
    * 初始化时为满的。
    * */
    pool->push_pos = numbers - 1;
    pthread_mutex_init(&pool->mutex,NULL);
    pthread_cond_init(&pool->empty,NULL);
#if 0
    switch(type)
    {
        case PACKET_POOL:
        packet_pool = pool;
        break;
        case FLOW_ITEM_POOL:
        packet_pool = pool;
        breakk;
        case SESSION_POOL:
        session_pool = pool;
        break;
        case BUFFER_POOL:
        buffer_pool = pool;
        break;
        case MANAGER_NODE_POOL:
        manager_node_pool = pool;
        break;
    }
#endif
    return pool;
}
/*
* 销毁一个缓冲区池子
* 时间：2014年5月8日17:49:21
* 作者: likeyi
* */
void destroy_pool(pool_t * pool)
{
    pthread_mutex_destroy(&pool->mutex); 
    pthread_cond_destroy(&pool->empty);
    free(pool->node);
    pool->node = NULL;
    free(pool->buffer);
    pool->buffer = NULL;
    free(pool);
    pool = NULL;
}
void free_buf(pool_t * pool,void * data)
{
    //pthread_mutex_lock(&pool->mutex);
    /*
    * 我们不测试pool是否已经满了。
    * 假如pool已经满了，现在仍然往pool写入
    * 那将是不可思议的事情。
    * */
	//printf("push_pos:%d\n",pool->push_pos);
    pool->node[pool->push_pos] = data;
    ++pool->push_pos;
    ++pool->free_num;
    if(pool->push_pos >= pool->total)
    {
        pool->push_pos = 0;
    }
    //pthread_cond_signal(&pool->empty);
    //pthread_mutex_unlock(&pool->mutex);
}
static inline void print_pool_type(pool_t * pool)
{
    switch(pool->pool_type)
    {
        case 0:printf("Packet Generator Pool\n");break;
        case 1:printf("Parser To Seeion Pool\n");break;
        case 2:printf("Session Pool\n");break;
        case 3:printf("Generator To Parser Queue\n");break;
        case 4:break;
    }
}
/*
*  本函数不用is_empty_pool，因为防止锁中锁
* */
int get_buf(pool_t * pool,int flag, void ** data)
{
    //pthread_mutex_lock(&pool->mutex);
    while(pool->push_pos == pool->pop_pos)
    {
        //print_pool_type(pool);
        if(flag == NO_WAIT_MODE)
        {
            //pthread_mutex_unlock(&pool->mutex);
            return -1; 
        }
        else
        {
            //pthread_cond_wait(&pool->empty,&pool->mutex);
        }
    }
    *data = pool->node[pool->pop_pos];
    ++pool->pop_pos;
    --pool->free_num;
    if(pool->pop_pos >= pool->total)
    {
        pool->pop_pos = 0;
    }
    //pthread_mutex_unlock(&pool->mutex);
    return 0;
}
/*
*  
* */
bool is_empty_pool(pool_t * pool)
{
    bool result;
    //pthread_mutex_lock(&pool->mutex);
    result = (pool->push_pos == pool->pop_pos);
    //pthread_mutex_unlock(&pool->mutex);
    return result;
}
bool is_full_pool(pool_t * pool)
{
    bool result;
    //pthread_mutex_lock(&pool->mutex);
    result =((pool->push_pos + 1) % pool->total == pool->pop_pos);
    //pthread_mutex_unlock(&pool->mutex);
    return result;
}
