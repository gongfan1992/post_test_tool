#ifndef __MPSOCK_H__
#define __MPSOCK_H__


#include <gxio/mpipe.h>
#include <tmc/cpus.h>


#define ALIGN(p, align) do { (p) += -(long)(p) & ((align) - 1); } while(0)

#define MP_DRIVERS                           1  //just one tile to work as a driver(one port)
#define EQUEUE_ENTRIES                       2048
#define NOTIF_RING_ENTRIES                   512
#define RINGS                                1
#define GROUPS                               1
#define BUCKETS                              1024
#define EDMARINGS                            1
#define STACKS                               1

#define ACK                                               0x06060606
#define COMMAND                                           0xF9F9F9F9
#define COMMAND_ACK                                       0xa1a1a1a1

#define MAX_PKT_SIZE                                      1024
#define MAX_SEND_SIZE                                     (1024 * 128)

// error defined
#define EMPSOCK_CORE                                      -1001
#define EMPSOCK_BIND                                      -1002
#define EMPSOCK_INSTANCE                                  -1003
#define EMPSOCK_GROUPS	                                  -1004
#define EMPSOCK_BUCKETS                                   -1005
#define EMPSOCK_EDMARINGS                                 -1006
#define EMPSOCK_RINGS					                            -1007
#define EMPSOCK_STACKS				                            -1008
#define EMPSOCK_COMMAND					                          -1009
#define EMPSOCK_INVALID                                   -1010


struct mplink {
    int cpu;                      // cpu on which the mpscok run
    char* link_name;              // link name of the network port
    int channel;                  // channel of the link
    int instance;
    int ring;
    int group;
    int bucket;
    int buffers;
    int equeue_entries;
    int stack_idx;
    int cookie;
    unsigned int edma;
    void* iqueue_mem;
    void* page;
    gxio_mpipe_bucket_mode_t mode;
    size_t notif_ring_entries;
    size_t notif_ring_size;
    size_t page_size;
    int iqueue_size;
    gxio_mpipe_link_t link;
    gxio_mpipe_context_t context;
    int stack;
    gxio_mpipe_iqueue_t* iqueue;
    gxio_mpipe_equeue_t* equeue;
    uint64_t rtotal;
    uint64_t stotal;        
};

int mpsock_link_open(struct mplink* ml, char* link_name);
int mpsock_context_init(struct mplink* ml);

int mpsock_send_data(struct mplink* ml, int size, char* data);
int mpsock_send(struct mplink* ml, int size);
int mpsock_recv_data(struct mplink* ml, char* data);
int mpsock_recv(struct mplink* ml);
int mpsock_get(struct mplink* ml);

#endif
