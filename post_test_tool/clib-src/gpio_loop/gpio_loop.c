#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "util.h"

#include <gxio/gpio.h>
#include <tmc/task.h>

uint64_t move_left(uint64_t* x, int count)
{
	*x = 0x1;
    uint32_t* p = (uint32_t*)(x);

    if (count < 32)
    {
        *(p + 1) <<= count;
        *(p + 1) |= (*p) >> (32 - count);
        *p <<= count;
    }
    else
    {
        *(p + 1) = *p;
        *p = 0x00000000;
        *(p + 1) <<= (count - 32);
    }  

	return *x;  
}

#define ASSERTED_PINS                           (1)

//static int asserted_output_pins[ASSERTED_PINS] = { 3, 1, 56, 57, 7, 23, 15 }; 
//static int asserted_input_pins[ASSERTED_PINS] = { 0, 2, 58, 59, 6, 22, 14 };
static int asserted_output_pins[ASSERTED_PINS] = { 52 }; 
static int asserted_input_pins[ASSERTED_PINS] = { 53 };

static void usage()
{
    printf("[option]:  --passes     Test times for the gpio test. Zero to test\n");
    printf("                        the gpio in loop\n");
}

int main(int argc, char** argv)
{
    int result = -1;
    int passes = 1;
    int counter = 0;
    char time_str[30];
    uint64_t asserted_output_mask = 0;
    uint64_t asserted_input_mask = 0;
    uint64_t pin_attach_mask = 0;
	uint64_t bitmap = 0x1;

    gxio_gpio_context_t gpio_context_body;
    gxio_gpio_context_t* gpio_context = &gpio_context_body;

    for (int i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        if (!strcmp(arg, "--passes") && i + 1 < argc)
        {
            passes = atoi(argv[++i]);
        }
        else
        {
            usage();
            return -1;
        }
    }

    for (int i = 0; i < ASSERTED_PINS; i++)
    {
		move_left(&bitmap, asserted_output_pins[i]);
        asserted_output_mask |= bitmap;
		move_left(&bitmap, asserted_input_pins[i]);
        asserted_input_mask |= bitmap;
    }

    pin_attach_mask = (asserted_output_mask | asserted_input_mask);

//    printf("0x%016lx, 0x%016lx, 0x%016lx\n", 
//            asserted_output_mask, asserted_input_mask, 
//            pin_attach_mask);

    result = gxio_gpio_init(gpio_context, 0);
    if (result)
    {
        get_time(time_str);
        printf("[GPIO-LOOP-%s]: Initialize the gpio module failed: 0x%016lx\n", 
                time_str, pin_attach_mask);
        return -1;
    }

    result = gxio_gpio_attach(gpio_context, pin_attach_mask);
    if (result)
    {
        if (result == GXIO_GPIO_ERR_PIN_UNAVAILABLE)
        {
            get_time(time_str);
            printf("[GPIO-LOOP-%s]: The gpio to be attached is unavaible: 0x%016lx\n",
                    time_str, pin_attach_mask);
            return -1;
        }
        else if (result == GXIO_GPIO_ERR_PIN_BUSY)
        {
            get_time(time_str);
            printf("[GPIO-LOOP-%s]: The gpio to be attached is busy: 0x%016lx\n",
                    time_str, pin_attach_mask);
            return -1;
        }
    }

    result = gxio_gpio_set_dir(gpio_context, 0, asserted_input_mask, 
                               asserted_output_mask, 0);
//    get_time(time_str);
//    printf("[GPIO-LOOP-%s]: Set the direction of the gpio failed: 0x%016lx(input), 0x%016lx(output)\n",
//                time_str, asserted_input_mask, 
//                asserted_output_mask);
    if (result)
    {   
        get_time(time_str);
        printf("[GPIO-LOOP-%s]: Set the direction of the gpio failed: 0x%016lx(input), 0x%016lx(output)\n",
                time_str, asserted_input_mask, 
                asserted_output_mask);
        return -1;
    }

    uint64_t input_val = 0;
    int error_mask[ASSERTED_PINS];
	int errors = 0;
        input_val = gxio_gpio_get(gpio_context);

//        printf("1Get the output value for asserted output test: 0x%016lx\n", 
//                input_val);


    while (counter != passes)
    {
        /** Pull up the output pin and check the input. */
        gxio_gpio_set(gpio_context, ~0, asserted_output_mask);

          input_val = gxio_gpio_get(gpio_context);
//        printf("2input_val = 0x%016lx\n", input_val);

        for (int i = 0; i < ASSERTED_PINS; i++)
        {
//			move_left(&bitmap, asserted_input_pins[i]);
			move_left(&bitmap, asserted_output_pins[i]);
            if ((input_val & bitmap) == 0)
            {
				move_left(&bitmap, asserted_input_pins[i]);
                error_mask[errors] = asserted_input_pins[i];
				errors++;
            } 
        }

        /** Pull down the output pin and check the input. */
        gxio_gpio_set(gpio_context, 0, asserted_output_mask);
        input_val = gxio_gpio_get(gpio_context);   

//        printf("3Get the output value for asserted output test: 0x%016lx, 0x%016lx\n", 
//                input_val, asserted_output_mask);
        for (int i = 0; i < ASSERTED_PINS; i++)
        {
           move_left(&bitmap, asserted_output_pins[i]);
//           move_left(&bitmap, asserted_input_pins[i]);
            if ((input_val & bitmap) != 0)
            {
                move_left(&bitmap, asserted_input_pins[i]);
                error_mask[errors] = asserted_input_pins[i];
                errors++;
            } 
        }     

		counter++;

		if (errors)
		{
            get_time(time_str);
            printf("[GPIO-LOOP-%s]: Loopback test on the gpio failed for the %d-nth time: ", 
                    time_str, counter);   
			for (int i = 0; i < errors; i++)
			{
				printf("%d ", error_mask[i]);
			}
			printf("\n");
            return -1;  
		}
    }

    get_time(time_str);
    printf("[GPIO-LOOP-%s]: Check the asserted output success for %d times.\n", 
            time_str, passes);

    return 0;
}
