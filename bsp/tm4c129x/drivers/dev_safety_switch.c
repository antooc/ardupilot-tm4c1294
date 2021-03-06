/*
 * dev_safty_switch.c
 *
 *  Created on: 2019Äê6ÔÂ3ÈÕ
 *      Author: Administrator
 */

#include "dev_safety_switch.h"
#include <rtthread.h>
#include <rtdevice.h>

/*----------------------------- Private define -------------------------------*/

#define BUTTON_PIN      45                      /* SW1 */
#define LED_PIN         41                      /* D1 */
#define BLINK_TIMER_TICK        1
#define BUTTON_TIMER_TICK       10
#define BUTTON_PRESS_TIMEOUT    300

/*----------------------------- Private typedef ------------------------------*/

typedef enum
{
    SAFETY_INIT = 0,
    SAFETY_DISARMED,
    SAFETY_ARMED,
} safety_switch_state_e;

typedef enum
{
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
    BUTTON_WAIT_RELEASE,
} button_state_e;

typedef struct
{
    rt_bool_t light;
    rt_tick_t delay;
} blink_item_t;

#define BLINK_END   {RT_FALSE, 0}

/*------------------------------ Private variables ---------------------------*/

static safety_switch_state_e state = SAFETY_INIT;

static blink_item_t blink_init[] = { {RT_TRUE, 10}, {RT_FALSE, 10}, BLINK_END};
static blink_item_t blink_disarmed[] = { {RT_TRUE, 10}, {RT_FALSE, 10}, {RT_TRUE, 10}, {RT_FALSE, 10}, {RT_TRUE, 10}, {RT_FALSE, 200}, BLINK_END };
static blink_item_t blink_armed[] = { {RT_TRUE, 100}, BLINK_END };

static blink_item_t *cur_blink_head = RT_NULL;
static blink_item_t *cur_blink = RT_NULL;
static rt_tick_t cur_blink_timeout = 0;

static rt_tick_t cur_button_timeout = 0;
static button_state_e button_state = BUTTON_RELEASED;

static rt_timer_t blink_timer = RT_NULL;
static rt_timer_t button_timer = RT_NULL;

/* ----------------------- Private function prototypes ---------------------*/

static void button_timer_entry(void *args);
static void blink_timer_entry(void *args);
static blink_item_t *next_blink_head(void);

/*-------------------------------- Functions -------------------------------*/

int dev_safety_switch_init(void)
{
    rt_pin_mode(BUTTON_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    button_timer = rt_timer_create("safty_button", button_timer_entry, RT_NULL, BUTTON_TIMER_TICK, RT_TIMER_FLAG_PERIODIC);
    if(!button_timer)
    {
        return -1;
    }
    rt_timer_start(button_timer);

    blink_timer = rt_timer_create("safty_blink", blink_timer_entry, RT_NULL, BLINK_TIMER_TICK, RT_TIMER_FLAG_PERIODIC);
    if(!blink_timer)
    {
        return -1;
    }
    rt_timer_start(blink_timer);

    return 0;
}
INIT_DEVICE_EXPORT(dev_safety_switch_init);

rt_bool_t dev_safety_switch_is_off(void)
{
    return state == SAFETY_ARMED;
}

void dev_safety_switch_set_ready(void)
{
    state = SAFETY_DISARMED;
}

/*-------------------------------- Private  Functions -------------------------------*/

static void button_timer_entry(void *args)
{
    if(state == SAFETY_INIT)
    {
        return;
    }

    rt_bool_t pressed = rt_pin_read(BUTTON_PIN) == PIN_LOW;
    if(button_state == BUTTON_RELEASED)
    {
        if(pressed)
        {
            cur_button_timeout = rt_tick_get() + BUTTON_PRESS_TIMEOUT;
            button_state = BUTTON_PRESSED;
        }
    }
    else if(button_state == BUTTON_PRESSED)
    {
        if(!pressed)
        {
            button_state = BUTTON_RELEASED;
        }
        else if(rt_tick_get() >= cur_button_timeout)
        {
            state = state == SAFETY_ARMED ? SAFETY_DISARMED : SAFETY_ARMED;
            button_state = BUTTON_WAIT_RELEASE;
        }
    }
    else if(button_state == BUTTON_WAIT_RELEASE)
    {
        if(!pressed)
        {
            button_state = BUTTON_RELEASED;
        }
    }
}

static void blink_timer_entry(void *args)
{
    blink_item_t *next_head = next_blink_head();

    if(!cur_blink_head || cur_blink_head != next_head)
    {
        cur_blink = cur_blink_head = next_head;
        cur_blink_timeout = 0;
    }

    if(cur_blink_timeout == 0)
    {
        cur_blink_timeout = rt_tick_get() + cur_blink->delay;
        rt_pin_write(LED_PIN, cur_blink->light ? PIN_HIGH : PIN_LOW);
        return;
    }

    if(rt_tick_get() < cur_blink_timeout)
    {
        return;
    }

    cur_blink++;
    if(cur_blink->delay == 0)
    {
        cur_blink = cur_blink_head;
    }
    cur_blink_timeout = rt_tick_get() + cur_blink->delay;
    rt_pin_write(LED_PIN, cur_blink->light ? PIN_HIGH : PIN_LOW);
}

static blink_item_t *next_blink_head(void)
{
    blink_item_t *next = RT_NULL;

    switch(state)
    {
    case SAFETY_INIT:
        next = blink_init;
        break;
    case SAFETY_ARMED:
        next = blink_armed;
        break;
    case SAFETY_DISARMED:
        next = blink_disarmed;
        break;
    default:
        next = blink_init;
        break;
    }

    return next;
}
