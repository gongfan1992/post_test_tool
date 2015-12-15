#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tmc/alloc.h>
#include <tmc/task.h>
#include <arch/cycle.h>
#include <gxio/mpipe.h>
#include <tmc/cpus.h>
#include <tmc/spin.h>
#include <tmc/sync.h>
#include <tmc/perf.h>
#include <arch/cycle.h>
#include <pthread.h>

#include "mpsock.h"
#include "util.h"

static char* link_name = "xgbe";
static int mode = -1;
uint64_t second = 0;
struct mplink ml;
volatile uint64_t counter = 0;

static void usage()
{
    printf("[option]:  --interface  Gigabit network port to be test.\n");
    printf("           --second     Seconds to test.\n");
}

int main(int argc, char** argv)
{
    int result = -1;    
    char time_str[30];
    char send_hand[64];
    char hand_ack[64];
    char shakehand[64];
    char* packet_send = malloc(512);
    char* packet_recv = malloc(512);

    for (int i = 0; i < 64; i++)
    {
        send_hand[i] = 't' + i % 26;
        hand_ack[i] = 't' - i % 26; 
    }

    for (int i = 0; i < 512; i++)
    {
        packet_send[i] = 'a' + i % 26;
    }
    
    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (!strcmp(arg, "--interface") && i + 1 < argc)
        {
            link_name = argv[++i];
        }
        else if (!strcmp(arg, "--second") && i + 1 < argc)
        {
            second = atoi(argv[++i]);
        }
        else if (!strcmp(arg, "--mode") && i + 1 < argc)
        {
            mode = atoi(argv[++i]);
        }
        else
        {
            usage();
            return -1;
        }
    }

    result = mpsock_link_open(&ml, link_name);
    if (result)
    {
        get_time(time_str);
        printf("[MPIPE-%s]: Can't open the link: %s\n", time_str, link_name);
        fflush(stdout);
        return -1;
    }

    result = mpsock_context_init(&ml);
    if (result)
    {
        get_time(time_str);
        printf("[MPIPE-%s]: Initialize the link failed: %s\n", time_str, link_name);
        fflush(stdout);
        return -1;
    }

    uint64_t start_cycle = get_cycle_count();
    uint64_t print_cycle = get_cycle_count();
    
    if (mode == 1)
    {
        // Start to send the shakehand packet.
        // If we can't finish the shakehand in 69 seconds just quit
        while (get_cycle_count() < (start_cycle + tmc_perf_get_cpu_speed() * 60))
        {
            result = mpsock_send_data(&ml, 64, send_hand);
            if (result == -1)
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s send one shakehand packet failed: %d.\n",
                        time_str, link_name, result);
                fflush(stdout);
                return -1;
            }
	    else if (result != -1 && result != 64)
	    {
		continue;
	    }

            result = mpsock_recv_data(&ml, shakehand);
            if (result == 64)
            {
                // Check if the received packet is shakehand ack.
                for (int i = 0; i < 64; i++)
                {
                    if (((unsigned char*)shakehand)[i] != 't' - i % 26)
                    {
                        get_time(time_str);
                        printf("[MPIPE-%s]: %s receive one erro ack packet.\n",
                                time_str, link_name);
                        fflush(stdout);
                        goto shakehand_err;
                    }
                }

                break;
            }
        }

        if (get_cycle_count() >= (start_cycle + tmc_perf_get_cpu_speed() * 60))
        {
            get_time(time_str);
            printf("[MPIPE-%s]: %s try to finish the shakehand failed in 60 seconds.\n",
                    time_str, link_name);
            fflush(stdout);
            goto shakehand_err;
        }
        else    
        {
            start_cycle = get_cycle_count();
            print_cycle = get_cycle_count();
            get_time(time_str);
            printf("[MPIPE-%s]: %s finish the shakehand start to transmit.\n",
                    time_str, link_name);
            fflush(stdout);
        }

        while (1)
        {
            result = mpsock_send_data(&ml, 512, packet_send);
            if (result == -1)
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s send one packet failed\n", time_str, link_name);
                fflush(stdout);
                return -1;
            }
	    else if (result != -1 && result != 512)
	    {
		continue;
	    }

            counter++;

            if (get_cycle_count() > (print_cycle + tmc_perf_get_cpu_speed() * 30))
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s transmit %lu packets success\n", 
                        time_str, link_name, counter);
                fflush(stdout);

                print_cycle = get_cycle_count();
            }
        }
    }
    else if (mode == 0)
    {
        // Start to receive the shakehand packet.
        // If we can't finish the shakehand in 60 seconds just quit
        while (get_cycle_count() < (start_cycle + tmc_perf_get_cpu_speed() * 60))
        {
            result = mpsock_recv_data(&ml, shakehand);
            if (result != 64)
            {
                continue;
            }
            else
            {
                // Check if the received packet is shakehand ack.
                for (int i = 0; i < 64; i++)
                {
                    if (((unsigned char*)shakehand)[i] != 't' + i % 26)
                    {
                        goto shakehand_err;
                    }
                }                
            }

            result = mpsock_send_data(&ml, 64, hand_ack);
            if (result != 64)
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s try to send the shakehand ack packet failed.\n",
                        time_str, link_name);
                fflush(stdout);
                return -1;
            }
            else
            {
                break;
            }
        }

        if (get_cycle_count() >= (start_cycle + tmc_perf_get_cpu_speed() * 60))
        {
            get_time(time_str);
            printf("[MPIPE-%s]: %s try to finish the shakehand failed in 60 seconds.\n",
                    time_str, link_name);
            fflush(stdout);
            goto shakehand_err;
        }
        else    
        {
            start_cycle = get_cycle_count();
            print_cycle = get_cycle_count();
            get_time(time_str);
            printf("[MPIPE-%s]: %s finish the shakehand start to transmit.\n",
                    time_str, link_name);
            fflush(stdout);
        }

        while (1)
        {
            result = mpsock_recv_data(&ml, packet_recv);
            if (result == -1)
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s receive one packet failed\n", time_str, link_name);
                fflush(stdout);
                return -1;
            }
	    else if (result != -1 && result != 512)
	    {
		continue;
	    }

            counter++;

            if (get_cycle_count() > (print_cycle + tmc_perf_get_cpu_speed() * 30))
            {
                get_time(time_str);
                printf("[MPIPE-%s]: %s transmit %lu packets success\n", 
                        time_str, link_name, counter);
                fflush(stdout);

                print_cycle = get_cycle_count();
            }
        }
    }

    return 0;
shakehand_err:
    return -1;
}

