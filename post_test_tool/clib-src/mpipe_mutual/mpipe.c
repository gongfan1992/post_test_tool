/*************************************************************************
	> File Name: mpipe.c
	> Author: likeyi
	> Mail: likeyi@sina.com 
	> Created Time: Tue 01 Jul 2014 11:11:47 AM CST
 ************************************************************************/

#include "includes.h"

static void create_stack(gxio_mpipe_context_t* context, int stack_idx,
             gxio_mpipe_buffer_size_enum_t buf_size, int num_buffers)
{
  int result;
  // Extract the actual buffer size from the enum.
  size_t size = gxio_mpipe_buffer_size_enum_to_buffer_size(buf_size);
  // Compute the total bytes needed for the stack itself.
  size_t stack_bytes = gxio_mpipe_calc_buffer_stack_bytes(num_buffers);
  // Round up so that the buffers will be properly aligned.
  stack_bytes += -(long)stack_bytes & (128 - 1);
  // Compute the total bytes needed for the stack plus the buffers.
  // 我看懂了，这是一个包含自己数据结构的buffer。
  // typedef struct statck
  // {
  //    XXX XX;
  //    XXX XX;
  //    char * p;
  //    XXX XX;
  // }statck_t
  size_t needed = stack_bytes + num_buffers * size;
  // Allocate up to 16 pages of the smallest suitable pagesize.
  tmc_alloc_t alloc = TMC_ALLOC_INIT;
  tmc_alloc_set_pagesize(&alloc, needed / 16);
  size_t pagesize = tmc_alloc_get_pagesize(&alloc);
  int pages = (needed + pagesize - 1) / pagesize;
  void* mem = tmc_alloc_map(&alloc, pages * pagesize);
  if (mem == NULL)
    tmc_task_die("Could not allocate buffer pages.");
  // Initialize the buffer stack.
  result = gxio_mpipe_init_buffer_stack(context, stack_idx, buf_size,
                                        mem, stack_bytes, 0);
  VERIFY(result, "gxio_mpipe_init_buffer_stack()");
  // Register the buffer pages.
  for (int i = 0; i < pages; i++)
  {
    result = gxio_mpipe_register_page(context, stack_idx,
                                      mem + i * pagesize, pagesize, 0);
    VERIFY(result, "gxio_mpipe_register_page()");
  }
  // Push the actual buffers.
  mem += stack_bytes;
  for (int i = 0; i < num_buffers; i++)
  {
    gxio_mpipe_push_buffer(context, stack_idx, mem);
    mem += size;
  }
}
static int init_mpipe_common(mpipe_common_t * mpipe)
{
    assert(mpipe);
    if(mpipe->configed == 0)
    {
        printf("mpipe must had configed\n");
        exit(-1);
    }

    int result;
    int link_flags = 0;

    gxio_mpipe_link_t link;
	gxio_mpipe_link_t send_link;
    gxio_mpipe_context_t * context = &mpipe->context;
    // 0. Determine the available cpus.
    result = tmc_cpus_get_my_affinity(&mpipe->cpus);
    VERIFY(result, "tmc_cpus_get_my_affinity()");

    if (tmc_cpus_count(&mpipe->cpus) < mpipe->num_workers)
        tmc_task_die("Insufficient cpus.");
    /* 
    * 1.Get the mPIPE instance for the link.
    * */
    char * link_name  = mpipe->recv_link_name;
    //printf("link name %s\n",link_name);
    int instance = gxio_mpipe_link_instance(link_name);
    if(instance < 0)
        tmc_task_die("Link '%s' does not exist.",link_name);


    /*
    * 2.Start the driver.
    * */
    result = gxio_mpipe_init(context,instance);
    VERIFY(result, "gxio_mpipe_init()");


    /* 
    * 3.Open the link 
    * */
	int recv_channel,send_channel;
	if(strcmp(mpipe->send_link_name,mpipe->recv_link_name) !=0)
	{
		result = gxio_mpipe_link_open(&link, context, link_name, link_flags);
		VERIFY(result, "gxio_mpipe_link_open()");

		link_name = mpipe->send_link_name;
		result = gxio_mpipe_link_open(&send_link, context, link_name,link_flags);
		/* 
		* 4.Get the channel 
		* */
		recv_channel = gxio_mpipe_link_channel(&link);
		send_channel = gxio_mpipe_link_channel(&send_link);
	
	}
	else
	{
		result = gxio_mpipe_link_open(&link, context, link_name, link_flags);
		VERIFY(result, "gxio_mpipe_link_open()");
		/* 
		* 4.Get the channel 
		* */
		recv_channel = gxio_mpipe_link_channel(&link);
		send_channel = recv_channel;

	}

	sleep(5);


    mpipe->instance = instance;
    mpipe->link     = link;
    mpipe->recv_channel  = recv_channel;
    mpipe->send_channel  = send_channel;


    return 0;
}

static int init_mpipe_iqueue(mpipe_common_t * mpipe)
{
    assert(mpipe);
    if(mpipe->configed == 0)
    {
        printf("mpipe must had configed\n");
        exit(-1);
    }
    int result;
    gxio_mpipe_iqueue_t** iqueues;
    gxio_mpipe_context_t * context = &mpipe->context;
    int num_workers = mpipe->num_workers;
    cpu_set_t cpus = mpipe->cpus;
    /* 
     * 1. Allocate some iqueues.
     */
    iqueues = calloc(num_workers, sizeof(*iqueues));
    if(iqueues == NULL)
        tmc_task_die("Failure in 'calloc()'.");

    /*
     * 2. Allocate some NotifRings.
     */
    result = gxio_mpipe_alloc_notif_rings(context, num_workers, 0, 0);
    VERIFY(result, "gxio_mpipe_alloc_notif_rings()");
    unsigned int ring = result;


    /* 
 	* 3. Init the NotifRings.
 	*/
    size_t notif_ring_entries = mpipe->notif_ring_entries;
    size_t notif_ring_size = notif_ring_entries * sizeof(gxio_mpipe_idesc_t);
    size_t needed = notif_ring_size + sizeof(gxio_mpipe_iqueue_t);
    for (int i = 0; i < num_workers; i++)
    {
        int cpu = tmc_cpus_find_nth_cpu(&cpus, i);
        tmc_alloc_t alloc = TMC_ALLOC_INIT;
        tmc_alloc_set_home(&alloc, cpu);
        // The ring must use physically contiguous memory, but the iqueue
        // can span pages, so we use "notif_ring_size", not "needed".
        tmc_alloc_set_pagesize(&alloc, notif_ring_size);
        void * iqueue_mem = tmc_alloc_map(&alloc, needed);
        if (iqueue_mem == NULL)
            tmc_task_die("Failure in 'tmc_alloc_map()'.");
        gxio_mpipe_iqueue_t* iqueue = iqueue_mem + notif_ring_size;
        result = gxio_mpipe_iqueue_init(iqueue, context, ring + i,
                                        iqueue_mem, notif_ring_size, 0);
        VERIFY(result, "gxio_mpipe_iqueue_init()");
        iqueues[i] = iqueue;
    }
    // 4. Allocate a NotifGroup.
    result = gxio_mpipe_alloc_notif_groups(context,num_workers, 0, 0);
    VERIFY(result, "gxio_mpipe_alloc_notif_groups()");
    int group = result;

    int num_buckets = mpipe->num_workers * mpipe->per_worker_buckets;
    result = gxio_mpipe_alloc_buckets(context, num_buckets, 0, 0);
    VERIFY(result, "gxio_mpipe_alloc_buckets()");
    int bucket = result;

    


    mpipe->iqueues = iqueues;
    mpipe->ring    = ring;
    mpipe->group   = group;
    mpipe->bucket  = bucket;
    mpipe->num_buckets = num_buckets;
    return 0;

}

static int init_mpipe_equeue(mpipe_common_t * mpipe)
{
    assert(mpipe);
    if(mpipe->configed == 0)
    {
        printf("mpipe must had configed\n");
        exit(-1);
    }
    int result;
    gxio_mpipe_context_t * context = &mpipe->context;
    int channel = mpipe->send_channel;
    gxio_mpipe_equeue_t *  equeue;
    equeue = malloc(sizeof(gxio_mpipe_equeue_t));
    if(equeue == NULL)
    {
        tmc_task_die("equeue malloc error\n");
    }
    /* 
    * 5.Initialize our edma ring.
    * 这里仅仅是分配eDMA rings,要记得分配buffer 
    * */ 
    result = gxio_mpipe_alloc_edma_rings(context, 1, 0, 0);
    VERIFY(result, "gxio_mpipe_alloc_edma_rings");
    uint edma = result;
    unsigned int equeue_entries = mpipe->equeue_entries;
    //printf("-----------edma rings %u--------\n",equeue_entries);
    size_t edma_ring_size = equeue_entries * sizeof(gxio_mpipe_edesc_t);
    tmc_alloc_t alloc = TMC_ALLOC_INIT;
    /* 分配页，并且map*/
    tmc_alloc_set_pagesize(&alloc,edma_ring_size);
    void * mem = tmc_alloc_map(&alloc,edma_ring_size);
    if (mem == NULL)
        tmc_task_die("Failed to allocate memory for the edma ring.");
    result = gxio_mpipe_equeue_init(equeue, context, edma,
            channel, mem, edma_ring_size, 0);
    VERIFY(result, "gxio_gxio_equeue_init()");


    /* 
     * 6.这里才是分配buffer 
     * */
    unsigned int num_bufs = equeue_entries + mpipe->num_workers * mpipe->notif_ring_entries;
	//printf("num_bufs:%d\n",num_bufs);
    result = gxio_mpipe_alloc_buffer_stacks(context, 1, 0, 0);
    VERIFY(result, "gxio_mpipe_alloc_buffer_stacks()");
    int stack_idx = result;
    create_stack(context,stack_idx, GXIO_MPIPE_BUFFER_SIZE_1664, num_bufs);

    mpipe->equeue = equeue;
    mpipe->stack_idx = stack_idx;

    return 0;
    
}

static int init_mpipe_rules(mpipe_common_t * mpipe)
{
	assert(mpipe);
    int result;
    if(mpipe->configed == 0)
    {
        printf("mpipe must had configed\n");
        exit(-1);
    }
    gxio_mpipe_context_t * context = &mpipe->context;
    int group         = mpipe->group;
    unsigned int ring = mpipe->ring;
    int bucket        = mpipe->bucket;
    int per_buckets   = mpipe->per_worker_buckets;
    gxio_mpipe_rules_dmac_t dmac = mpipe->dmac;
    gxio_mpipe_rules_dmac_t temp_dmac;



	gxio_mpipe_rules_t rules;
	gxio_mpipe_rules_init(&rules, context);
    	gxio_mpipe_bucket_mode_t mode = GXIO_MPIPE_BUCKET_ROUND_ROBIN; 
	int i = 0;
	for(i = 0; i < mpipe->num_workers; i++)
	{
                temp_dmac = dmac;
        	/*
        	* 可以起到每个线程一个ring
        	* 每个线程per_workers_buckets个buckets的效果吗？
        	* */
        	gxio_mpipe_init_notif_group_and_buckets(context,
                                               group + i,
                                               ring + i,
                                               1,
                                               bucket + i * per_buckets, 
                                               per_buckets, 
                                               mode);
        	gxio_mpipe_rules_begin(&rules,
                               bucket + i * per_buckets,
                               per_buckets,
                               NULL);
        	temp_dmac.octets[5] = dmac.octets[5] + i;
        	gxio_mpipe_rules_add_dmac(&rules,temp_dmac);
	}
    result = gxio_mpipe_rules_commit(&rules);
    VERIFY(result, "gxio_mpipe_rules_commit()");

    return 0;
		
}

int init_mpipe_config(mpipe_common_t * mpipe,config_t * config)
{
    assert(mpipe);
    assert(config);

    mpipe->send_link_name = malloc(strlen(config->send_link_name) + 1);
    assert(mpipe->send_link_name);
    strcpy(mpipe->send_link_name,config->send_link_name);


    mpipe->recv_link_name = malloc(strlen(config->send_link_name) + 1);
    assert(mpipe->recv_link_name);
    strcpy(mpipe->recv_link_name,config->recv_link_name);
    mpipe->notif_ring_entries = config->notif_ring_entries;
    mpipe->equeue_entries     = config->equeue_entries;
    mpipe->per_worker_buckets = config->per_worker_buckets;
    mpipe->num_workers        = config->num_workers;
    mpipe->dmac               = config->dmac;

    mpipe->configed = 1;
    return 0;
}

// The main function for each worker thread.
void init_mpipe_resource(mpipe_common_t * mpipe)
{

    assert(mpipe);

    init_mpipe_common(mpipe);

    init_mpipe_iqueue(mpipe);

    init_mpipe_equeue(mpipe);

    init_mpipe_rules(mpipe);


}
void mpipe_send_packet(mpipe_common_t * mpipe,
                       uint16_t size, 
                       void * packet)
{

	gxio_mpipe_context_t * context = &mpipe->context;

    gxio_mpipe_equeue_t * equeue = mpipe->equeue;

    int stack_idx = mpipe->stack_idx;
	
	void * buffer = gxio_mpipe_pop_buffer(context,stack_idx);
	if(buffer == NULL)
		tmc_task_die("Could not pop seed buffer!");
	memcpy(buffer, packet, size);
	__insn_mf(); 
    gxio_mpipe_edesc_t edesc = 
    {
        {
            .bound = 1,
            .xfer_size = size,
            .va = (uintptr_t)buffer,
            .stack_idx = stack_idx,
	        .hwb  = 1,	
            .size = GXIO_MPIPE_BUFFER_SIZE_1664
        }
    };
    gxio_mpipe_equeue_put(equeue, edesc);
}
