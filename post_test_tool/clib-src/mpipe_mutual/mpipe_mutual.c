/*************************************************************************
	> File Name: sim_top.c
	> Author: likeyi
	> Mail: likeyiyy@sina.com 
	> Created Time: Tue 13 May 2014 04:07:01 PM CST
 ************************************************************************/

#include "includes.h"
extern generator_set_t * generator_set;
int top_argc = 0;
char ** top_argv;

static   const char * config_file = CONFIG_FILE;

int g_index = 0;
int g_passes = 1;

static void usage()
{
	printf("[option]: --second          Seconds to test.\n");
	printf("          --sender          The sender of the mpipe mutual test.\n");
	printf("          --recver          The recver of the mpipe mutual test.\n");
	printf("          --config          The config file of the mpipe mutual test.\n");
    printf("          --errornum        设置错误统计次数，超过此次数后退出.\n");
    printf("          --timeout         设置等待一个包的时间，超时后不等待.\n");
    printf("          --queue_length    设置一次发包的数量.\n");
    printf("          --len_check       无参数，校验包长度.\n");
    printf("          --data_check      无参数，校验包数据.\n");
    printf("          --version         无参数，显示版本信息\n");
    printf("          --help            帮助信息\n");
    
}
int main(int argc, char ** argv)
{
    config_t * config = malloc(sizeof(config_t));
    signal(SIGINT,print_counter_info);
	for(int i = 1; i < argc; i++)
	{
		char * arg = argv[i];
		if (!strcmp(arg, "--config") && i + 1 < argc)
		{
			config_file = argv[++i];
		}
    }

    exit_if_ptr_is_null(config,"config error");
    read_config_file(config_file, config);

	for(int i = 1; i < argc; i++)
	{
		char * arg = argv[i];
		if (!strcmp(arg, "--second") && i + 1 < argc)
		{
			g_passes = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "--sender") && i + 1 < argc)
		{
			config->send_link_name = argv[++i];
		}
		else if (!strcmp(arg, "--recver") && i + 1 < argc)
		{
			config->recv_link_name = argv[++i];
		}
		else if (!strcmp(arg, "--config") && i + 1 < argc)
		{
			config_file = argv[++i];
		}
		else if (!strcmp(arg, "--srcmac") && i + 1 < argc)
		{
            ether_atoe(argv[++i],config->srcmac);
		}
		else if (!strcmp(arg, "--errornum") && i + 1 < argc)
		{
			config->errornum = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "--pktlen") && i + 1 < argc)
		{
			config->pktlen = atoi(argv[++i]);
		}
        else if(!strcmp(arg, "--workers") && i + 1 < argc)
        {
            config->num_workers = atoi(argv[++i]);
        }
		else if (!strcmp(arg, "--timeout") && i + 1 < argc)
		{
			config->timeout = atol(argv[++i]);
		}
		else if (!strcmp(arg, "--queue_length") && i + 1 < argc)
		{
            config->once_packet_nums = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "--check_level") && i + 1 < argc)
		{
            		config->check_level = atoi(argv[++i]);
		}
		else if (!strcmp(arg, "--version") )
		{
            printf("[MPIPE-MUTUAL]: version: V1.30\n");
            return 0;
		}
		else if (!strcmp(arg, "--help") )
		{
			usage();
			return -1;
		}
		else
		{
			usage();
			return -1;
		}	
	}	
    print_config_file(config);
    /* Generator */
    init_generator_set(config);

    tmc_cpus_set_my_cpu(0);
    while(1)
    {
        sleep(2);
        fflush(stdout);
    }
    pthread_exit(NULL);
}
