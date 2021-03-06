/*
 * hrtimer_test.c
 *
 *  Created on: 2019Äê4ÔÂ17ÈÕ
 *      Author: Administrator
 */

#include "hrtimer_test.h"

#include <rtthread.h>
#include <rtdevice.h>

#include <drv_hrtimer.h>
#include <hr_timer.h>

#define LED_PIN1    0
#define LED_PIN2    1

typedef struct
{
    char name[RT_NAME_MAX];
    rt_uint32_t pin;
    rt_uint32_t period;

}led_t;

static led_t led_array[] =
{
        {
                .name = "led1",
                .pin = LED_PIN1,
                .period = 1250,
        },

        {
                .name = "led2",
                .pin = LED_PIN2,
                .period = 2500,
        }
};

static void timeout_entry(void *args)
{
    led_t *led = (led_t *)args;
    rt_pin_write(led->pin, !rt_pin_read(led->pin));
//    rt_kprintf("l\n");
}

void hrtimer_test(void)
{
    rt_uint32_t last_tick = 0;
    rt_uint32_t cur_tick;

    for(int i = 0; i < sizeof(led_array) / sizeof(led_array[0]); i++)
    {
        led_t *led = led_array + i;
        rt_pin_mode(led->pin, PIN_MODE_OUTPUT);
        rt_pin_write(led->pin, PIN_HIGH);

        hrtimer_t led_timer = hrtimer_create(led->name, timeout_entry, led, led->period, RT_TIMER_FLAG_PERIODIC);
        if(!led_timer)
        {
            rt_kprintf("Fail to create led_timer\n");
            return;
        }
        hrtimer_start(led_timer);
    }

    while(1)
    {
        cur_tick = (rt_uint32_t)hrtimer_get_tick();
        rt_kprintf("%9d, %9d\n", cur_tick, cur_tick - last_tick);
        last_tick = cur_tick;
        rt_thread_delay(100);
    }
}
