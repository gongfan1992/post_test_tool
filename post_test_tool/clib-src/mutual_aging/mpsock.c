#include "mpsock.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tmc/alloc.h>
#include <tmc/cpus.h>
#include <tmc/task.h>
#include <arch/cycle.h>
#include <tmc/perf.h>

//uint8_t global_trace_mask = DEFAULT_TRACE;
uint8_t global_trace_mask = 0;

/** open one link 
 * @param: ml mplink of the mpipe to initialize
 * @param: link_name name of the port, which should be open
 * @param: rank on which tile to run
 * @return: ZERO on success, or one negative error code
 */
int mpsock_link_open(struct mplink* ml, char* link_name)
{
    int result;
    gxio_mpipe_context_t* context = &ml->context;

    // get the instance
    ml->instance = gxio_mpipe_link_instance(link_name);
    if (ml->instance < 0)
    {
        return EMPSOCK_INSTANCE;
    }
    
    // Start the driver
    result = gxio_mpipe_init(context, ml->instance);
        
    result = gxio_mpipe_link_open(&ml->link, context, link_name, 0);

    sleep(3);
    
    ml->channel = gxio_mpipe_link_channel(&ml->link);
    ml->link_name = link_name;

    // Set the mac loop mode.
    //result = gxio_mpipe_link_set_attr(&ml->link, GXIO_MPIPE_LINK_DESIRED_STATE, GXIO_MPIPE_LINK_LOOP_PHY);

    return result;   
}

/** allocate resource and initialize the port
 * @param: ml mplink initialized by mpsock_link_open
 * @return: ZERO on success, or one negative error code
 */
int mpsock_context_init(struct mplink* ml)
{
    int result;
    gxio_mpipe_context_t* context = &ml->context;
    
    // Allocate NotifRing
    result = gxio_mpipe_alloc_notif_rings(context, RINGS, 0, 0);
    if (result < 0)
    {
        return EMPSOCK_RINGS;
    }
    ml->ring = result;
    
    // Init the NotifRing, every iqueue memory contain num*idesc + iqueueself    
    ml->notif_ring_entries = NOTIF_RING_ENTRIES;
    size_t notif_ring_size = ml->notif_ring_entries * sizeof(gxio_mpipe_idesc_t);
    size_t needed = notif_ring_size + sizeof(gxio_mpipe_iqueue_t);
  
    tmc_alloc_t ialloc = TMC_ALLOC_INIT;
    tmc_alloc_set_pagesize(&ialloc, notif_ring_size);
    void* iqueue_mem = tmc_alloc_map(&ialloc, needed);
    assert(iqueue_mem);

    gxio_mpipe_iqueue_t* iqueue = iqueue_mem + notif_ring_size;
    assert(iqueue);
    result = gxio_mpipe_iqueue_init(iqueue, context, ml->ring,
                                    iqueue_mem, notif_ring_size, 0);
    assert(!result);
    ml->iqueue = iqueue;

    tmc_alloc_t alloc = TMC_ALLOC_INIT;
    tmc_alloc_set_huge(&alloc);
    size_t page_size = tmc_alloc_get_pagesize(&alloc);
    void* page = tmc_alloc_map(&alloc, page_size);
    assert(page);  
   
    void* mem = page;
    
    // Allocate a NotifGroup.
    result = gxio_mpipe_alloc_notif_groups(context, GROUPS, 0, 0);
    if (result < 0)
    {
        return EMPSOCK_GROUPS;
    }
    ml->group = result;
    
    // Allocate some buckets.
    result = gxio_mpipe_alloc_buckets(context, BUCKETS, 0, 0);
    if (result < 0)
    {
        return EMPSOCK_BUCKETS;
    }    
    ml->bucket = result;
    
    ml->mode = GXIO_MPIPE_BUCKET_DYNAMIC_FLOW_AFFINITY;
    result = gxio_mpipe_init_notif_group_and_buckets(context, ml->group,
                                                     ml->ring, MP_DRIVERS,
                                                     ml->bucket, BUCKETS, ml->mode);    
    assert(!result);

    // Allocate an edma ring.
    result = gxio_mpipe_alloc_edma_rings(context, EDMARINGS, 0, 0);
    if (result < 0)
    {
        return EMPSOCK_EDMARINGS;
    } 
    ml->edma = result;
    
    // Init edma ring
    ml->equeue = (gxio_mpipe_equeue_t*)malloc(sizeof(gxio_mpipe_equeue_t));
    assert(ml->equeue);
    
    size_t edma_ring_size = EQUEUE_ENTRIES * sizeof(gxio_mpipe_edesc_t);
    result = gxio_mpipe_equeue_init(ml->equeue, context, ml->edma, ml->channel,
                                  mem, edma_ring_size, 0);
    assert(!result);
    mem += edma_ring_size;
    ml->equeue_entries = EQUEUE_ENTRIES;
                            
    ml->buffers = ml->equeue_entries + MP_DRIVERS * ml->notif_ring_entries;
    
    result = gxio_mpipe_alloc_buffer_stacks(context, STACKS, 0, 0);
    if (result < 0)
    {
        return EMPSOCK_STACKS;
    }
    ml->stack_idx = result;
    
    // Initialize the buffer stack.
    ALIGN(mem, 0x10000);
    size_t stack_bytes = gxio_mpipe_calc_buffer_stack_bytes(ml->buffers);
    gxio_mpipe_buffer_size_enum_t buf_size = GXIO_MPIPE_BUFFER_SIZE_1664;
    result = gxio_mpipe_init_buffer_stack(context, ml->stack_idx, buf_size,
                                        mem, stack_bytes, 0);
    assert(!result);
    mem += stack_bytes;

    ALIGN(mem, 0x10000);
    
    // Register the entire huge page of memory which contains all the buffers.
    result = gxio_mpipe_register_page(context, ml->stack_idx, page, page_size, 0);
    assert(!result);
    
    // Push some buffers onto the stack.
    for (int i = 0; i < ml->buffers; i++)
    {
        gxio_mpipe_push_buffer(context, ml->stack_idx, mem);
        mem += 1664;
    }

    // Paranoia.
    assert(mem <= page + page_size);
    
    gxio_mpipe_rules_t rules;
    gxio_mpipe_rules_init(&rules, context);
    gxio_mpipe_rules_begin(&rules, ml->bucket, BUCKETS, NULL);
    gxio_mpipe_rules_add_channel(&rules, ml->channel);
    result = gxio_mpipe_rules_commit(&rules);
    assert(!result);
    
    return 0;
    
}

/** send data though network port
 * @param: ml mplink initialized before
 * @param: buf pointer of the data which to send
 * @param: size the size of data which to send, can't greater than MAX_SEND_SIZE
 * @return: number of data(bytes) has sent
 */
int mpsock_send(struct mplink* ml, int size)
{
    gxio_mpipe_context_t* context = &ml->context;
    void* mp_buf = NULL;
    
    // transfer size check
    if (size < 64 || size > 1024)
        return -1;
    
    while (mp_buf == NULL)    
    {
        mp_buf = gxio_mpipe_pop_buffer(context, ml->stack_idx);
    }
	
	unsigned char* tmp = (unsigned char*)mp_buf;
	for (int i = 0; i < size; i++)
	{
		tmp[i] = 'a' + i % 26;
	}
         
    gxio_mpipe_edesc_t edesc = {{
        .bound = 1,
        .xfer_size = size,
        .va = (uintptr_t)mp_buf,
        .stack_idx = ml->stack_idx,
        .inst = ml->instance,
        .hwb = 1,
        .size = GXIO_MPIPE_BUFFER_SIZE_1664,
    }};
    gxio_mpipe_equeue_put(ml->equeue, edesc);    

    return size;
}

/** send data though network port
 * @param: ml mplink initialized before
 * @param: buf pointer of the data which to send
 * @param: size the size of data which to send, can't greater than MAX_SEND_SIZE
 * @return: number of data(bytes) has sent
 */
int mpsock_send_data(struct mplink* ml, int size, char* data)
{
    gxio_mpipe_context_t* context = &ml->context;
    void* mp_buf = NULL;
    
    // transfer size check
    if (size < 64 || size > 1024)
        return -1;
    
    while (mp_buf == NULL)    
    {
        mp_buf = gxio_mpipe_pop_buffer(context, ml->stack_idx);
    }

    memcpy(mp_buf, data, size);
         
    gxio_mpipe_edesc_t edesc = {{
        .bound = 1,
        .xfer_size = size,
        .va = (uintptr_t)mp_buf,
        .stack_idx = ml->stack_idx,
        .inst = ml->instance,
        .hwb = 1,
        .size = GXIO_MPIPE_BUFFER_SIZE_1664,
    }};
    gxio_mpipe_equeue_put(ml->equeue, edesc);    

    return size;
}

/** receive data though network port
 * @param: ml mplink initialized before
 * @param: buf pointer of the data where to put, the buffer should be larger than 128K
 * @return: number of data(bytes) has received
 */
int mpsock_recv(struct mplink* ml)
{
    int result = -1;
    uint64_t timeout = get_cycle_count();
    gxio_mpipe_idesc_t idesc;
    
    while (result != 0)
    {
        result = gxio_mpipe_iqueue_try_get(ml->iqueue, &idesc);
        if (get_cycle_count() > (timeout + tmc_perf_get_cpu_speed()))
        {
            return -1;
        }
    }

    result = gxio_mpipe_idesc_get_xfer_size(&idesc);
    gxio_mpipe_iqueue_drop(ml->iqueue, &idesc);
     
    return result; 
}

/** receive data though network port
 * @param: ml mplink initialized before
 * @param: buf pointer of the data where to put, the buffer should be larger than 128K
 * @return: number of data(bytes) has received
 */
int mpsock_recv_data(struct mplink* ml, char* data)
{
    int result = -1;
    uint64_t timeout = get_cycle_count();
    gxio_mpipe_idesc_t idesc;
    
    while (result != 0)
    {
        result = gxio_mpipe_iqueue_try_get(ml->iqueue, &idesc);
        if (get_cycle_count() > (timeout + tmc_perf_get_cpu_speed()))
        {
            return -1;
        }
    }

    result = gxio_mpipe_idesc_get_xfer_size(&idesc);
    memcpy(data, gxio_mpipe_idesc_get_va(&idesc), result);
    
    gxio_mpipe_iqueue_drop(ml->iqueue, &idesc);
     
    return result; 
}

int mpsock_get(struct mplink* ml)
{
    int result = -1;
    gxio_mpipe_idesc_t idesc;

    gxio_mpipe_iqueue_get(ml->iqueue, &idesc);

    result = gxio_mpipe_idesc_get_xfer_size(&idesc);
    gxio_mpipe_iqueue_drop(ml->iqueue, &idesc);
     
    return result; 
}
