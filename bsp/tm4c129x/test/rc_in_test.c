/*
 * rc_in_test.c
 *
 *  Created on: 2019Äê5ÔÂ13ÈÕ
 *      Author: Administrator
 */

#include "rc_in_test.h"
#include <rtthread.h>
#include "dev_rc_in.h"

#define PULSE_NUM   8

static void print_pulse(rt_uint16_t *pulses, rt_size_t len);

void rc_in_test(void)
{
    rc_in_data_t rc_in_data;

    while(1)
    {
        rt_memset(&rc_in_data, 0, sizeof(rc_in_data));
        dev_rc_in_read(&rc_in_data);
        if(rc_in_data.channel_count < PULSE_NUM)
        {
            rt_kprintf("dev_rc_in_read fails, channel_count=%d\n", rc_in_data.channel_count);
        }
        else
        {
            print_pulse(rc_in_data.values, rc_in_data.channel_count);
        }

        rt_thread_delay(5);
    }
}

static void print_pulse(rt_uint16_t *pulses, rt_size_t len)
{
    for(int i = 0; i < len; i++)
    {
        rt_kprintf("% 4d  ", pulses[i]);
    }
    rt_kprintf("\n");
}
