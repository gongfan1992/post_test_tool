/*************************************************************************
	> File Name: packet_generator.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Wed 30 Apr 2014 03:02:16 PM CST
 ************************************************************************/
#include <tmc/perf.h>
#include "includes.h"

generator_set_t * generator_set;
extern pool_t * packet_pool;

static int exit_noti_flag    = 0;
//static int exit_ok_flag      = 0;

static int finish_counter    = 0;
pthread_mutex_t finish_lock;

#define LENGTH_MATCH 0x1
#define DATA_MATCH   0x2

static void get_time(char *str_t)
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

static uint32_t get_cpu_mhz()
{
    int mhz;
    int i = 0;
    uint64_t new,old;
    int delayms = 500;                            
    int total = 0;
    int loop_num = 4;
    for(i = 0; i < loop_num; i++)                      
    {                                             
        old = GET_CYCLE_COUNT();                  
        usleep(delayms * 1000);                   
        new = GET_CYCLE_COUNT();                  
        total += (new - old) / (delayms * 1000);
    }
    mhz = total/loop_num;
    return mhz;
}
#define NS 1000000000
uint32_t calc_period(double length,double rate,uint32_t thread_num)
{
     uint32_t mhz = get_cpu_mhz();
     printf("cpu mhz %d\n",mhz);
     double l = 8.0 * NS * length;
     double x = rate * 1024 * 1024.0;
     return  (uint32_t)((mhz / 1000.0) * (thread_num * l / x));
}
void   destroy_generator(generator_set_t * generator_set)
{
    int i = 0;
    for(i = 0; i < generator_set->numbers; ++i)
    {
        generator_set->generator[i].config     = NULL;
        generator_set->generator[i].pool       = NULL;
    }
    free(generator_set->generator);
    generator_set->generator = NULL;
    /* 销毁缓冲区池子 */
    /*
     * 销毁配置文件分配的内存。
     * */
    free(generator_set->config->pkt_data);
    generator_set->config->pkt_data = NULL;
    free(generator_set->config);
    generator_set->config = NULL;
    free(generator_set);
    generator_set = NULL;
}
void   finish_generator(generator_set_t * generator_set)
{
    int i = 0;
    for(i = 0; i < generator_set->numbers; ++i)
    {
        pthread_cancel(generator_set->generator[i].id);
    }
}
int pop_payload(void * payload, char * data,config_t * config)
{
	unsigned char * pay = (unsigned char *)payload;
    int j ;
	if(config->protocol == IPPROTO_TCP)
	{
		j = config->pktlen - 54;
	}
	else
	{
		j = config->pktlen - 42;
	}

	int k = 0;
	while(j--)
	{
		pay[k] = rand()%256;
		k++;
	}
#if 0
    if(i > j)
    {
        memcpy(payload,data,j);
    }
    int start = 0;
    while(j > i)
    {
        memcpy(payload + start,data,i);
        start += i;
        j -= i;
    }
    memcpy(payload + start,data,j);
    /*
    * 返回payload的长度。
    * */
#endif
    return config->pktlen - 54;
}
#define GET_NEXT_SRCIP(config) \
        ((config->saddr_cur == config->saddr_max) ? \
         (config->saddr_cur = config->saddr_min) : \
         (++config->saddr_cur))
static inline uint32_t get_next_srcip(config_t * config)
{
    return config->saddr_cur == config->saddr_max ?
    config->saddr_cur = config->saddr_min:
    ++config->saddr_cur;
}
#define GET_NEXT_DSTIP(config) \
        ((config->daddr_cur == config->daddr_max) ? \
         (config->daddr_cur = config->daddr_min) : \
         (++config->daddr_cur))
static inline uint32_t get_next_dstip(config_t * config)
{
    return config->daddr_cur == config->daddr_max ?
    config->daddr_cur = config->daddr_min:
    ++config->daddr_cur;
}
#define GET_NEXT_SRCPORT(config) \
        ((config->sport_cur == config->sport_max) ? \
         (config->sport_cur = config->sport_min) : \
         (++config->sport_cur))
static inline uint16_t get_next_srcport(config_t * config)
{
    return config->sport_cur == config->sport_max ?
    config->sport_cur = config->sport_min:
    ++config->sport_cur;
}
#define GET_NEXT_DSTPORT(config) \
        ((config->dport_cur == config->dport_max) ? \
         (config->dport_cur = config->dport_min) : \
         (++config->dport_cur))
static inline uint16_t get_next_dstport(config_t * config)
{
    return config->dport_cur == config->dport_max ?
    config->dport_cur = config->dport_min:
    ++config->dport_cur;
}
static inline int pop_transmission_tcp(void * tcph,config_t * config)
{
    struct tcphdr * tcp = (struct tcphdr *)tcph;
    tcp->source = htons(GET_NEXT_SRCPORT(config));
    tcp->dest   = htons(GET_NEXT_DSTPORT(config));
    tcp->doff  = sizeof(struct tcphdr) / 4;
    tcp->check = 0;
    return config->pktlen - 34;
}
static inline int pop_transmission_udp(void * udph,config_t * config)
{
    struct udphdr * udp = (struct udphdr *)udph;
    udp->source = htons(GET_NEXT_SRCPORT(config));
    udp->dest   = htons(GET_NEXT_DSTPORT(config));
    udp->len    = htons(config->pktlen - 34);
    udp->check  = 0;
    return config->pktlen - 34;
}
static inline void pop_iplayer_tcp(void * iph,config_t * config)
{
    struct iphdr * ip = (struct iphdr *)iph;
    ip->version = IPVERSION;
    ip->ihl     = sizeof(struct iphdr) / sizeof(uint32_t);
    ip->tot_len = htons(config->pktlen-14);
    ip->ttl     = IPDEFTTL;
    ip->protocol = IPPROTO_TCP;
    ip->saddr   = htonl(GET_NEXT_SRCIP(config));
    ip->daddr   = htonl(GET_NEXT_DSTIP(config));
    ip->check   = ~ip_xsum((uint16_t *)ip,sizeof(struct iphdr)/2,0);
    /*
    * Do TCP header Check Sum
    * */
    struct tcphdr * tcp = (struct tcphdr *)((unsigned char *)ip+20);
    uint16_t sum = 0x6 + config->pktlen - 34;
    tcp->check = (~ip_xsum((uint16_t *)&ip->saddr,(config->pktlen-26)/2,htons(sum)));
}
static inline void pop_iplayer_udp(void * iph,config_t * config)
{
    struct iphdr * ip = (struct iphdr *)iph;
    ip->version = IPVERSION;
    ip->ihl     = sizeof(struct iphdr) / sizeof(uint32_t);
    ip->tot_len = htons(config->pktlen-14);
    ip->ttl     = IPDEFTTL;
    ip->protocol = IPPROTO_UDP;
    ip->saddr   = htonl(GET_NEXT_SRCIP(config));
    ip->daddr   = htonl(GET_NEXT_DSTIP(config));
    ip->check   = ~ip_xsum((uint16_t *)ip,sizeof(struct iphdr)/2,0);
    /*
    * Do UDP header Check Sum
    * */
    struct udphdr * udp = (struct udphdr *)((unsigned char *)ip+20);
    uint16_t sum = 0x17 + config->pktlen - 34;
    udp->check = (~ip_xsum((uint16_t *)((unsigned char *)ip+12),(config->pktlen-26)/2,htons(sum)));
}
static inline void pop_datalink(void * packet,config_t * config)
{
    struct ethhdr * eth_hdr = (struct ethhdr *)(packet);
    memcpy(eth_hdr->h_dest,config->dstmac,ETH_ALEN);
    memcpy(eth_hdr->h_source,config->srcmac,ETH_ALEN);
    eth_hdr->h_proto = htons(ETH_P_IP);
}
typedef void (GenerHandler) (void * packet,config_t * config);
static inline void generator_tcp_packet(void * packet,config_t * config)
{
    //pop_payload(packet->data+54,config->pkt_data,config);
    pop_transmission_tcp(packet + 34, config);
    pop_iplayer_tcp(packet + 14,config);
    pop_datalink(packet,config);
}

static inline void generator_udp_packet(void * packet,config_t * config)
{
    pop_payload(packet+42,config->pkt_data,config);
    pop_transmission_udp(packet+34,config);
    pop_iplayer_udp(packet+14,config);
    pop_datalink(packet,config);
}

static inline void free_packet(packet_t * packet) 
{
        free_buf(packet->pool,packet);
}

static int compare_packet(unsigned char * p1,unsigned char * p2,int length)
{
    int i = 0;
    for(i = 0; i < length; i++)
    {
        if(*p1++ != *p2++)
        {
			//printf("------------%d %X %X\n",i,*p1,*p2);
            return -1;
        }
    }
    return 0;
}


static void make_all_packet(generator_t * generator,GenerHandler * Handler)
{
	config_t * config = generator->config;
	packet_t * packet;
    int rank = generator->rank;
	for(int i = 0; i < config->once_packet_nums;i++)
	{
			/*
			* 1. get buffer from pool
			* */
			if(get_buf(generator->pool,NO_WAIT_MODE,(void **)&packet) < 0)
			{
				generator->drop_total++;
				break;
			}
			packet->pool   = generator->pool;
			packet->length = config->pktlen;
			packet->data   = (unsigned char *)packet + sizeof(packet_t);
			/*
			* 2. 根据配置文件比如UDP，TCP来产生包结构。
			* */
			Handler(packet->data,config);
			struct ethhdr * eth_hdr = (struct ethhdr *)(packet->data);
			eth_hdr->h_dest[5] += rank;	
			packet->crc = *(uint64_t *)(packet->data+42);
			free_packet(packet);
	}
}

static inline void pass_or_exit(uint64_t error_len,uint64_t error_data,uint64_t error_recv,config_t * config)
{
    if((error_len >= config->errornum) ||(error_data >= config->errornum) || (error_recv >= config->errornum))
    {
        printf("Error cause length miss match: %lu \
                    Error cause data miss match: %lu \
                    Error cause recv miss: %lu \n",
                  error_len, error_data, error_recv);
            exit(0);
    }
}
static void generator_packet(generator_t * generator,int data_len,GenerHandler * Handler)
{
    packet_t * packet;
    int result = -1;
    mpipe_common_t * mpipe = generator->mpipe_config;
    //int rank = generator->rank;
	//gxio_mpipe_context_t * context = &mpipe->context;
    //int notif_ring_entries = mpipe->notif_ring_entries;
	
	srand((unsigned int)time(NULL));
	
    gxio_mpipe_iqueue_t * iqueue = mpipe->iqueues[generator->rank];
    config_t * config = generator->config;
    gxio_mpipe_idesc_t idesc;

	make_all_packet(generator,Handler);

	//printf("config check level: %x \n",config->check_level);

	uint64_t start_cycle = get_cycle_count();
	uint64_t interval = tmc_perf_get_cpu_speed() * (uint64_t)g_passes;
	while (get_cycle_count() < (start_cycle + interval))
    {
        if(!exit_noti_flag)
        {
    		for(int i = 0; i < config->once_packet_nums;i++)
    		{
    			if(get_buf(generator->pool,NO_WAIT_MODE,(void **)&packet) < 0)
    			{
    				generator->drop_total++;
    				//printf("buf is empty\n");
    				break;
    			}
    			/*
    			* 3. 数据输出到网口
    			* */
    			mpipe_send_packet(mpipe, config->pktlen, packet->data);
    			generator -> total_send_byte += config -> pktlen;
    			if(push_session_buf(generator->queue, packet) != true)
    			{
    				break;
    			}
    		}
    
    		for(int i = 0; i < config->once_packet_nums;i++)
    		{
             /*
             * 4. read back and compare
             * */
    			packet_t * head;
                uint64_t timeout = get_cycle_count();
    			uint64_t crc;
                result = -1;
                timeout = get_cycle_count();
                while (result != 0)
                {
    			    result = gxio_mpipe_iqueue_try_get(iqueue,&idesc);
                    if (get_cycle_count() > (timeout + config->timeout))
                    {
                        break;
                    }
                }
    			if(result == 0)
    			{
    				//printf("-----------------after mpipe read %d--------------------\n",rank);
    				unsigned char * read_back =  gxio_mpipe_idesc_get_va(&idesc);
    				//uint32_t xfer_size = gxio_mpipe_idesc_get_xfer_size(&idesc);
    				uint32_t l2_size = gxio_mpipe_idesc_get_l2_length(&idesc);
    
    				//printf("-------xfer_size:%u ||| l2_length:%u",xfer_size,l2_size);
    				if(pop_session_buf(generator->queue,(void **)&head) != true)
    				{
    					break;
    				}
    				crc = *(uint64_t *)(read_back + 42);
                    if(config -> check_level & LENGTH_MATCH)
    				{
                        if(head->length != l2_size)
                        {
                            generator->error_len++;
                            pass_or_exit(generator->error_len,
                                         generator->error_data,
                                         generator->error_recv,config);
                        }
    					else
    					{
    						generator->correct_len++;	
    					}
                    }
                    if(config -> check_level & DATA_MATCH )
                    {
                        if(compare_packet(head->data,
                                          read_back,
                                          config->pktlen))
    		    		{
                            generator->error_data++;
                            pass_or_exit(generator->error_len,
                                         generator->error_data,
                                         generator->error_recv,
                                         config);
    		    		}
    					else
    					{
    						generator->correct_data++;	
    					}
                    }
                    free_buf(head->pool,head);
    				gxio_mpipe_iqueue_drop(iqueue, &idesc);
    			}
    			else
    			{
    				if(pop_session_buf(generator->queue,
                                       (void **)&head) != true)
    				{
    					break;
    				}
                    free_buf(head->pool, head);
                    generator->error_recv++;
                    pass_or_exit(generator->error_len,generator->error_data,generator->error_recv,config);
    			}
    		}
        }
    }
    pthread_mutex_lock(&finish_lock);
    ++finish_counter;
    pthread_mutex_unlock(&finish_lock);
}
void * packet_generator_loop(void * arg)
{
    pthread_detach(pthread_self());
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
    generator_t * generator = (generator_t *)arg;
    mpipe_common_t * mpipe = generator->mpipe_config;
    config_t * config = generator->config;
    int rank = generator->rank;

    /**
     * Bind to a single cpu
     * */
    int cpu = tmc_cpus_find_nth_cpu(&mpipe->cpus, rank);
    //printf("rank %d cpu %d\n",rank,cpu);
    int result = tmc_cpus_set_my_cpu(cpu);
    VERIFY(result, "tmc_cpus_set_my_cpu()");
    
    //printf("--before wait barrier %d\n",rank);
    tmc_sync_barrier_wait(&mpipe->barrier);
    //printf("--after wait barrier %d\n",rank);

    int data_len = config->pktlen + sizeof(packet_t);
    /*
    * UDP MODE
    * */
    generator_packet(generator,data_len,generator_udp_packet);

    pthread_exit(NULL);
}

void   print_counter_info(int sig)
{
	uint64_t error_len    = 0;
	uint64_t error_data   = 0;
	uint64_t correct_len  = 0;
	uint64_t correct_data = 0;
	uint64_t error_recv   = 0;
	for(int i = 0; i < generator_set->numbers; i++)
	{
		error_len 		+= generator_set -> generator[i].error_len;
		error_data 		+= generator_set -> generator[i].error_data;
		correct_len 	+= generator_set -> generator[i].correct_len;
		correct_data 	+= generator_set -> generator[i].correct_data;
		error_recv 		+= generator_set -> generator[i].error_recv;
	}
    printf("Error cause length miss match: %lu \nError cause data miss match:   %lu \nError cause recv miss:       %lu\nCorrect len : 		%lu \ncorrect data: 		%lu \n",
                  error_len, error_data, error_recv,correct_len,correct_data);
	if((error_len == 0)&&(error_data == 0)&& (error_recv == 0)) 
	{
		printf("%d second SUCCESSED,Exit Now\n",g_passes);
	}
	else
	{
		printf("%d second FAILED,   Exit Now\n",g_passes);
	}
    exit_noti_flag = 1;
    sleep(1);
    exit(0);
}
void * counter_loop(void * arg)
{
	generator_set_t * generator_set = (generator_set_t * )arg;
	mpipe_common_t * mpipe = generator_set->generator[0].mpipe_config;
	//int rank = generator_set->numbers;
	int i = 0;
	uint64_t new;
	uint64_t old = 0;
	struct timeval new_time,old_time;
    char time_str[30];
	uint64_t error_len    = 0;
	uint64_t error_data   = 0;
	uint64_t correct_len  = 0;
	uint64_t correct_data = 0;
	uint64_t error_recv   = 0;
    /**
     * Bind to a single cpu
     * */
    int result = tmc_cpus_set_my_cpu(0);
    VERIFY(result, "tmc_cpus_set_my_cpu()");

    tmc_sync_barrier_wait(&mpipe->barrier);

	gettimeofday(&old_time,NULL);
	while (1)
	{
		new = 0;
		error_len 	 = 0;
		error_data 	 = 0;
		correct_len  = 0;
		correct_data = 0;
		error_recv 	 = 0;
		sleep(1);
		for(i = 0; i < generator_set->numbers; i++)
		{
			new += generator_set -> generator[i].total_send_byte;
		}
        get_time(time_str);
		gettimeofday(&new_time, NULL);
		uint64_t ms = new_time.tv_sec * 1000 + new_time.tv_usec / 1000 - (old_time.tv_sec * 1000 + old_time.tv_usec / 1000);	
        if(!exit_noti_flag)
        {
		    printf("[MPIPE-MUTUAL-%s-%s]-----%lu bit/ms %lf Gbps ",generator_set->link_name, time_str,(new-old) * 8 / ms,(new - old) * 8.0/(ms * 1000 * 1024));
        }
		for(i = 0; i < generator_set->numbers; i++)
		{
			error_len 		+= generator_set -> generator[i].error_len;
			error_data 		+= generator_set -> generator[i].error_data;
			correct_len 	+= generator_set -> generator[i].correct_len;
			correct_data 	+= generator_set -> generator[i].correct_data;
			error_recv 		+= generator_set -> generator[i].error_recv;
		}
    	printf("ELENGTH:%lu EDATA: %lu ERECV:%lu\n",error_len, error_data, error_recv);
		old = new;
		old_time = new_time;
        if(finish_counter == generator_set->numbers )
        {
            break;
        }
	}
	for(i = 0; i < generator_set->numbers; i++)
	{
		error_len 		+= generator_set -> generator[i].error_len;
		error_data 		+= generator_set -> generator[i].error_data;
		correct_len 	+= generator_set -> generator[i].correct_len;
		correct_data 	+= generator_set -> generator[i].correct_data;
		error_recv 		+= generator_set -> generator[i].error_recv;
	}
    printf("Error cause length miss match: %lu \nError cause data miss match:   %lu \nError cause recv miss:       %lu\nCorrect len : 		%lu \ncorrect data: 		%lu \n",
                  error_len, error_data, error_recv,correct_len,correct_data);
	if((error_len == 0)&&(error_data == 0)&& (error_recv == 0)) 
	{
		printf("%d second SUCCESSED,Exit Now\n",g_passes);
	    exit(-1);
	}
	else
	{
		printf("%d second FAILED,   Exit Now\n",g_passes);
	    exit(0);
	}


}
void init_generator_set(config_t * config)
{
    int i = 0;
    int numbers = config->num_workers;

    generator_set = malloc(sizeof(generator_set_t));

    /*
     * 初始化一个缓冲区池。
     * 这个缓冲区的头部是个结构体指针，下面是packet_length的长度的缓冲区。
     * */
    generator_set->generator = malloc(sizeof(generator_t) * numbers);
    exit_if_ptr_is_null(generator_set->generator,"---------------generator_set.generator error-------------------");
    generator_set->numbers   = numbers;
    generator_set->config    = config;
	
	generator_set->link_name = malloc(strlen(config->send_link_name) + 1);
	assert(generator_set->link_name);
	strcpy(generator_set->link_name,config->send_link_name);


    mpipe_common_t * mpipe = malloc(sizeof(mpipe_common_t));

    exit_if_ptr_is_null(mpipe,"--------------------------malloc mpipe error-------------------");

    init_mpipe_config(mpipe,config);

    init_mpipe_resource(mpipe);    

    tmc_sync_barrier_init(&mpipe->barrier,mpipe->num_workers+1);

    pthread_mutex_init(&finish_lock, NULL);

    for(i = 0; i < numbers; i++)
    {
        generator_set->generator[i].pool = init_pool(PACKET_POOL,
                                                    config->once_packet_nums + 1,
                                                    config->pktlen + sizeof(packet_t));
        generator_set->generator[i].pool->pool_type = PACKET_POOL;
		generator_set->generator[i].queue = init_manager_queue(config->once_packet_nums + 1,config->pktlen);
        
        generator_set->generator[i].config = malloc(sizeof(config_t)); 
        exit_if_ptr_is_null(generator_set->generator[i].config,"config error");
        memcpy(generator_set->generator[i].config,config,sizeof(config_t));

        generator_set->generator[i].index = i;
        generator_set->generator[i].next_thread_id = 0;
        generator_set->generator[i].total_send_byte = 0;
        generator_set->generator[i].rank = i + g_index;
        //generator_set->generator[i].mpipe_config = malloc(sizeof(mpipe_common_t));
        //exit_if_ptr_is_null(generator_set->generator[i].mpipe_config,"mpipe_config");
        //memcpy(generator_set->generator[i].mpipe_config,mpipe,sizeof(mpipe_common_t));

	    generator_set->generator[i].mpipe_config = mpipe;
        
        if(pthread_create(&generator_set->generator[i].id,
                      NULL,
                      packet_generator_loop,
                      &generator_set->generator[i]) != 0)
        {
            printf("Init Packet Generator thread failed. Exit Now.\n");
            exit(0);
        }
    }
	pthread_t counter_thread;
	pthread_create(&counter_thread,NULL,counter_loop,(void *)generator_set);
}
