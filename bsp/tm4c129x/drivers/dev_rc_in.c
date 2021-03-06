/*
 * dev_rc_in.c
 *
 *  Created on: 2019Äê5ÔÂ10ÈÕ
 *      Author: Administrator
 */

#include "dev_rc_in.h"

#include "board.h"
#include "hr_timer.h"


/*----------------------------- Private define -------------------------------*/

#define RC_IN_PERIPH        SYSCTL_PERIPH_GPIOC
#define RC_IN_PORT          GPIO_PORTC_BASE
#define RC_IN_PIN           GPIO_PIN_6
#define RC_IN_INT_PIN       GPIO_INT_PIN_6
#define RC_IN_INT_HANDLER       GPIOC_IRQHandler

#define END_PULSE           (2500)
#define RC_LOST_DELAY       (1000000)
/*----------------------------- Private typedef ------------------------------*/

typedef enum
{
    MATCH_END_POSITIVE = 0,
    MATCH_END_NEGATIVE,
    MATCH_POSITIVE,
    MATCH_NEGATIVE,
}parse_state_e;

/*------------------------------ Private variables ---------------------------*/

static rc_in_data_t rc_in_data = {0};

/* temporary data */
static parse_state_e cur_state = MATCH_END_POSITIVE;
static rt_uint32_t pulse_index = 0;
static rt_uint16_t pulse_array_temp[RC_IN_CH_NUM];
static hr_tick_t last_tick = 0;
static rt_uint32_t last_negative_pulse = 0;

/* ----------------------- Private function prototypes ---------------------*/
static void pin_init(void);
static void pin_int_handler(bool is_positive);

static void data_init(void);
static void check_lost(void);
/*-------------------------------- Functions -------------------------------*/

int dev_rc_in_init(void)
{
    data_init();
    pin_init();

    return 0;
}
INIT_DEVICE_EXPORT(dev_rc_in_init);

void dev_rc_in_read(rc_in_data_t *data)
{
    register rt_base_t level;

    check_lost();

    level = rt_hw_interrupt_disable();
    rt_memcpy(data, &rc_in_data, sizeof(rc_in_data));
    rt_hw_interrupt_enable(level);
}

void RC_IN_INT_HANDLER(void)
{
    uint32_t status;

    status = GPIOIntStatus(RC_IN_PORT, true);
    GPIOIntClear(RC_IN_PORT, status);

    if(status & RC_IN_INT_PIN)
    {
        pin_int_handler(GPIOPinRead(RC_IN_PORT, RC_IN_PIN) != 0);
    }
}

/*-------------------------------- Private  Functions -------------------------------*/

static void pin_init(void)
{
    SysCtlPeripheralEnable(RC_IN_PERIPH);
    while(!SysCtlPeripheralReady(RC_IN_PERIPH));

    GPIOIntRegister(RC_IN_PORT, RC_IN_INT_HANDLER);
    GPIOIntTypeSet(RC_IN_PORT, RC_IN_PIN, GPIO_BOTH_EDGES);
    GPIOPinTypeGPIOInput(RC_IN_PORT, RC_IN_PIN);
    GPIOIntEnable(RC_IN_PORT, RC_IN_INT_PIN);
}

static void pin_int_handler(bool is_positive)
{
    rt_uint32_t pulse = 0;
    rt_tick_t cur_tick = hrtimer_get_tick();;
    pulse = cur_tick - last_tick;
    last_tick = cur_tick;

    switch(cur_state)
    {
    case MATCH_END_POSITIVE:
        if(is_positive)
        {
            cur_state = MATCH_END_NEGATIVE;
        }
        break;
    case MATCH_END_NEGATIVE:
        if(is_positive)
        {
            goto RESET;
        }
        if(pulse < END_PULSE)
        {
            goto RESET;
        }
        pulse_index = 0;
        cur_state = MATCH_POSITIVE;
        break;
    case MATCH_POSITIVE:
        if(!is_positive)
        {
            goto RESET;
        }
        cur_state = MATCH_NEGATIVE;
        last_negative_pulse = pulse;
        break;
    case MATCH_NEGATIVE:
        if(is_positive)
        {
            goto RESET;
        }
        if(pulse >= END_PULSE)
        {
            goto COPY_DATA;
        }
        pulse_array_temp[pulse_index++] = pulse + last_negative_pulse;
        if(pulse_index >= RC_IN_CH_NUM)
        {
            goto COPY_DATA;
        }
        cur_state = MATCH_POSITIVE;
        break;
    default:
        break;
    }
    return;

COPY_DATA:
    if(pulse_index < RC_IN_CH_NUM)
    {
        for(int i = pulse_index; i < RC_IN_CH_NUM; i++)
        {
            pulse_array_temp[i] = 0;
        }
        register rt_base_t level;
        level = rt_hw_interrupt_disable();
        rc_in_data.channel_count = pulse_index;
        rc_in_data.rc_failsafe = false;
        rc_in_data.timestamp_last_signal = hrtimer_get_tick();
        rt_memcpy(rc_in_data.values, pulse_array_temp, sizeof(rc_in_data.values));
        rt_hw_interrupt_enable(level);
    }

RESET:
    cur_state = MATCH_END_POSITIVE;
    last_tick = 0;
    last_negative_pulse = 0;
    return;

}

static void data_init(void)
{
    rt_memset(&rc_in_data, 0, sizeof(rc_in_data));
}

static void check_lost(void)
{
    register rt_base_t level;
    uint64_t cur_time = hrtimer_get_tick();

    level = rt_hw_interrupt_disable();
    if(rc_in_data.channel_count && !rc_in_data.rc_failsafe
            && cur_time - rc_in_data.timestamp_last_signal >= RC_LOST_DELAY)
    {
        rc_in_data.rc_failsafe = true;
    }
    rt_hw_interrupt_enable(level);
}
