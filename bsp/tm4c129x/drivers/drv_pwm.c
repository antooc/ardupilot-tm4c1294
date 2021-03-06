/*
 * drv_pwm.c
 *
 *  Created on: 2019年8月2日
 *      Author: Administrator
 */

#include "drv_pwm.h"

#include "board.h"
#include "driverlib/tiva_timer.h"

/*----------------------------- Private define -------------------------------*/



/*----------------------------- Private typedef ------------------------------*/

typedef struct
{
    rt_uint32_t timer_periph;
    rt_uint32_t timer_base;
    rt_uint32_t timer_part;
    rt_uint32_t timer_cfg;

    rt_uint32_t gpio_periph;
    rt_uint32_t gpio_port;
    rt_uint32_t gpio_pin;
    rt_uint32_t gpio_pin_cfg;

    rt_uint32_t frequency;
    rt_uint32_t period;
    rt_uint32_t pulse;
} pwm_t;

/*------------------------------ Private variables ---------------------------*/

static pwm_t pwm_array[] =
{
     /* PM1(T2CCP1) */
     {
          .timer_periph = SYSCTL_PERIPH_TIMER2,
          .timer_base = TIMER2_BASE,
          .timer_part = TIMER_B,
          .timer_cfg = TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM,

          .gpio_periph = SYSCTL_PERIPH_GPIOM,
          .gpio_port = GPIO_PORTM_BASE,
          .gpio_pin = GPIO_PIN_1,
          .gpio_pin_cfg = GPIO_PM1_T2CCP1,
     }
};

/* ----------------------- Private function prototypes ---------------------*/

static void pwm_init(pwm_t *pwm);
static void pwm_lock(pwm_t *pwm);
static void pwm_unlock(pwm_t *pwm);
static void pwm_update(pwm_t *pwm);

/*-------------------------------- Functions -------------------------------*/


int drv_pwm_init(void)
{
    for(int i = 0; i < sizeof(pwm_array) / sizeof(pwm_array[0]); i++)
    {
        pwm_init(pwm_array + i);
    }
//    drv_pwm_set_frequency(0, 2000);

    return 0;
}
INIT_BOARD_EXPORT(drv_pwm_init);

void drv_pwm_set_frequency(int index, rt_uint32_t frequency)
{
    if(index < 0 || index >= sizeof(pwm_array) / sizeof(pwm_array[0]))
    {
        return;
    }

    pwm_t *pwm = pwm_array + index;
    pwm->frequency = frequency;
    if(pwm->frequency == 0)
    {
        pwm->period = pwm->pulse = UINT32_MAX;
    }
    else
    {
        pwm->period = SystemCoreClock / pwm->frequency;
        if(pwm->period > 0x1000000)
        {
            pwm->period = 0x1000000;
        }
        pwm->pulse = pwm->period / 2;
    }
    pwm_update(pwm);
}

/*-------------------------------- Private  Functions -------------------------------*/

static void pwm_init(pwm_t *pwm)
{
    /*
     * configure gpio
     */
    SysCtlPeripheralEnable(pwm->gpio_periph);
    while (!SysCtlPeripheralReady(pwm->gpio_periph));
    /* 先锁定，之后根据系统情况解锁 */
    pwm_lock(pwm);

    /*
     * configure timer
     */
    SysCtlPeripheralEnable(pwm->timer_periph);
    while (!SysCtlPeripheralReady(pwm->timer_periph));
    TimerConfigure(pwm->timer_base, pwm->timer_cfg);
    TimerEnable(pwm->timer_base, pwm->timer_part);

}

static void pwm_lock(pwm_t *pwm)
{
    GPIOPinTypeGPIOOutput(pwm->gpio_port, pwm->gpio_pin);
    GPIOPinWrite(pwm->gpio_port, pwm->gpio_pin, 0xff);
}

static void pwm_unlock(pwm_t *pwm)
{
    GPIOPinConfigure(pwm->gpio_pin_cfg);
    GPIOPinTypeTimer(pwm->gpio_port, pwm->gpio_pin);
}

static void pwm_update(pwm_t *pwm)
{
    if(pwm->frequency)
    {
        uint32_t value;

        value = pwm->period - 1;
        TimerPrescaleSet(pwm->timer_base, pwm->timer_part, value >> 16);
        TimerLoadSet(pwm->timer_base, pwm->timer_part, value & 0xffff);

        value = pwm->pulse - 1;
        TimerPrescaleMatchSet(pwm->timer_base, pwm->timer_part, value >> 16);
        TimerMatchSet(pwm->timer_base, pwm->timer_part, value & 0xffff);
        pwm_unlock(pwm);
    }
    else
    {
        pwm_lock(pwm);
    }
}
