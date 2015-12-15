/*************************************************************************
	> File Name: mpipe.h
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Tue 01 Jul 2014 11:08:31 AM CST
 ************************************************************************/

#ifndef L_MPIPE_H
#define L_MPIPE_H


typedef struct __mpipe_t
{
    /*
     * Config 
     * */
    int         configed;
    char *      send_link_name;
	char *      recv_link_name;
    int         notif_ring_entries;
    uint32_t    equeue_entries;
    int         per_worker_buckets;
    int         num_workers;
    gxio_mpipe_rules_dmac_t dmac;
    /*
     * sys
     * */
    tmc_sync_barrier_t barrier; 
    cpu_set_t cpus;
    /*
     * Common
     * */
    int instance;
    gxio_mpipe_link_t link;
    int send_channel;
	int recv_channel;
    gxio_mpipe_context_t   context;

    /*
     * Ingress
     * */
    gxio_mpipe_iqueue_t **  iqueues;
    unsigned int            ring;
    int                     group;
    int                     bucket;
    int                     num_buckets;

    /*
     * Egress
     * */
    gxio_mpipe_equeue_t *  equeue;
    unsigned int edma;
    int stack_idx;

}mpipe_common_t;

int init_mpipe_config(mpipe_common_t * mpipe,config_t * config);

void init_mpipe_resource(mpipe_common_t * mpipe);

void mpipe_send_packet(mpipe_common_t * mpipe,
                        uint16_t sizes, 
                        void * packets);

extern char * link_name;

#endif
