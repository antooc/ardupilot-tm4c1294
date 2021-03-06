/*
 * gpio_test.c
 *
 *  Created on: 2019Äê4ÔÂ17ÈÕ
 *      Author: Administrator
 */

#include "gpio_test.h"
#include <rtthread.h>
#include <rtdevice.h>

#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

static const int led_pin_array[] = {0, 1, 2, 3};
#define USER_SW_1_PIN   4

void gpio_test(void)
{
    int index = 0;

    for(int i = 0; i < ITEM_NUM(led_pin_array); i++)
    {
        rt_pin_mode(led_pin_array[i], PIN_MODE_OUTPUT);
        rt_pin_write(led_pin_array[i], PIN_LOW);
    }
    rt_pin_mode(USER_SW_1_PIN, PIN_MODE_INPUT_PULLUP);

    while(1)
    {
        while(!rt_pin_read(USER_SW_1_PIN));

        rt_pin_write(led_pin_array[index], PIN_HIGH);
        rt_thread_delay(10);

        rt_pin_write(led_pin_array[index], PIN_LOW);
        index++;
        if(index >= ITEM_NUM(led_pin_array))
        {
            index = 0;
        }
    }
}
