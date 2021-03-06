/*
 * dev_rc_out.c
 *
 *  Created on: 2019年5月10日
 *      Author: Administrator
 */

#include "dev_rc_out.h"

#include "board.h"
#include "driverlib/pwm.h"
#include "dev_safety_switch.h"

/*----------------------------- Private define -------------------------------*/

#define PWM_BASE                PWM0_BASE
#define PWM_PERIOD_DEFAULT      2500
#define PWM_PULSE_DEFAULT       1000
#define CHECK_LOCK_PERIOD       10

/*----------------------------- Private typedef ------------------------------*/

typedef struct
{
    rt_uint32_t pwm_gen;
    rt_uint32_t pwm_out;
    rt_uint32_t pwm_out_bit;

    rt_uint32_t gpio_periph;
    rt_uint32_t gpio_port;
    rt_uint32_t gpio_pin;
    rt_uint32_t gpio_pin_cfg;

    rt_uint32_t period;
    rt_uint32_t pulse;
} pwm_out_t;

/*------------------------------ Private variables ---------------------------*/

/* in microseconds */
static float pwm_clk_period;

static pwm_out_t pwm_out_array[] =
{
    /* PF1(M0PWM1) */
    {
         .pwm_gen = PWM_GEN_0,
         .pwm_out = PWM_OUT_1,
         .pwm_out_bit = PWM_OUT_1_BIT,

         .gpio_periph = SYSCTL_PERIPH_GPIOF,
         .gpio_port = GPIO_PORTF_BASE,
         .gpio_pin = GPIO_PIN_1,
         .gpio_pin_cfg = GPIO_PF1_M0PWM1,
    },

    /* PF2(M0PWM2) */
    {
         .pwm_gen = PWM_GEN_1,
         .pwm_out = PWM_OUT_2,
         .pwm_out_bit = PWM_OUT_2_BIT,

         .gpio_periph = SYSCTL_PERIPH_GPIOF,
         .gpio_port = GPIO_PORTF_BASE,
         .gpio_pin = GPIO_PIN_2,
         .gpio_pin_cfg = GPIO_PF2_M0PWM2,
    },

    /* PF3(M0PWM3) */
    {
         .pwm_gen = PWM_GEN_1,
         .pwm_out = PWM_OUT_3,
         .pwm_out_bit = PWM_OUT_3_BIT,

         .gpio_periph = SYSCTL_PERIPH_GPIOF,
         .gpio_port = GPIO_PORTF_BASE,
         .gpio_pin = GPIO_PIN_3,
         .gpio_pin_cfg = GPIO_PF3_M0PWM3,
    },

    /* PG0(M0PWM4) */
    {
         .pwm_gen = PWM_GEN_2,
         .pwm_out = PWM_OUT_4,
         .pwm_out_bit = PWM_OUT_4_BIT,

         .gpio_periph = SYSCTL_PERIPH_GPIOG,
         .gpio_port = GPIO_PORTG_BASE,
         .gpio_pin = GPIO_PIN_0,
         .gpio_pin_cfg = GPIO_PG0_M0PWM4,
    },
};

static volatile bool is_locked = false;

/* ----------------------- Private function prototypes ---------------------*/
static void pwm_init(void);
static void pwm_out_init(pwm_out_t *pwm_out);
static rt_err_t pwm_set_period(pwm_out_t *pwm_out, rt_uint32_t period);
static rt_err_t pwm_set_pulse(pwm_out_t *pwm_out, rt_uint32_t pulse);
static void pwm_lock_all(void);
static void pwm_unlock_all(void);
static void pwm_lock(pwm_out_t *pwm_out);
static void pwm_unlock(pwm_out_t *pwm_out);

static void check_timer_entry(void *args);
static bool check_lock(void);

/*-------------------------------- Functions -------------------------------*/

int dev_rc_out_init(void)
{
    pwm_init();
    for(int i = 0; i < sizeof(pwm_out_array) / sizeof(pwm_out_array[0]); i++)
    {
        pwm_out_t *pwm_out = pwm_out_array + i;
        pwm_out->period = PWM_PERIOD_DEFAULT;
        pwm_out->pulse = PWM_PULSE_DEFAULT;
        pwm_out_init(pwm_out);
    }

    rt_timer_t timer = rt_timer_create("rc_out", check_timer_entry, RT_NULL, CHECK_LOCK_PERIOD, RT_TIMER_FLAG_PERIODIC);
    if(!timer)
    {
        return -RT_ERROR;
    }
    rt_timer_start(timer);


    return 0;
}
INIT_DEVICE_EXPORT(dev_rc_out_init);

void dev_rc_out_set_period(rt_uint16_t period)
{
    rt_err_t err;
    pwm_out_t *pwm_out;

    for(int i = 0; i < sizeof(pwm_out_array) / sizeof(pwm_out_array[0]); i++)
    {
        pwm_out = pwm_out_array + i;
        err = pwm_set_period(pwm_out, period);
        if(err != RT_EOK)
        {
            continue;
        }
        pwm_set_pulse(pwm_out, pwm_out->pulse);
    }
}

void dev_rc_out_write(rt_uint16_t *pulses, rt_size_t len)
{
    pwm_out_t *pwm_out;

    for(int i = 0; i < len && i < sizeof(pwm_out_array) / sizeof(pwm_out_array[0]); i++)
    {
        pwm_out = pwm_out_array + i;
        pwm_set_pulse(pwm_out, pulses[i]);
    }
}

/*-------------------------------- Private  Functions -------------------------------*/

static void pwm_init(void)
{
    //
    // Enable the PWM0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    //
    // Wait for the PWM0 module to be ready.
    //
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));

    PWMClockSet(PWM_BASE, PWM_SYSCLK_DIV_64);
    pwm_clk_period = 1.0f / (SystemCoreClock / 1000000 / 64.0f);

}

static void pwm_out_init(pwm_out_t *pwm_out)
{
    /*
     * configure gpio
     */
    SysCtlPeripheralEnable(pwm_out->gpio_periph);
    while (!SysCtlPeripheralReady(pwm_out->gpio_periph));
//    GPIOPinConfigure(pwm_out->gpio_pin_cfg);
//    GPIOPinTypePWM(pwm_out->gpio_port, pwm_out->gpio_pin);
    /* 先锁定，之后根据系统情况解锁 */
    pwm_lock(pwm_out);

    /*
     * configure pwm gen and out
     */
    PWMGenConfigure(PWM_BASE, pwm_out->pwm_gen, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    pwm_set_period(pwm_out, pwm_out->period);
    pwm_set_pulse(pwm_out, pwm_out->pulse);
    PWMGenEnable(PWM_BASE, pwm_out->pwm_gen);
    PWMOutputState(PWM_BASE, pwm_out->pwm_out_bit, true);
}

static rt_err_t pwm_set_period(pwm_out_t *pwm_out, rt_uint32_t period)
{
    rt_uint32_t pwm_cnt;

    pwm_cnt = (rt_uint32_t)(period / pwm_clk_period);
    if(pwm_cnt > 0xffff)
    {
        return -RT_ERROR;
    }

    pwm_out->period = period;
    PWMGenPeriodSet(PWM_BASE, pwm_out->pwm_gen, pwm_cnt);
    return RT_EOK;
}

static rt_err_t pwm_set_pulse(pwm_out_t *pwm_out, rt_uint32_t pulse)
{
    rt_uint32_t ccp_cnt;

    if(pulse >= pwm_out->period)
    {
        return -RT_ERROR;
    }

    ccp_cnt = pulse * PWMGenPeriodGet(PWM_BASE, pwm_out->pwm_gen) / pwm_out->period;
    ccp_cnt = ccp_cnt > 0 ? ccp_cnt : 1;
    PWMPulseWidthSet(PWM_BASE, pwm_out->pwm_out, ccp_cnt);
    pwm_out->pulse = pulse;

    return RT_EOK;
}

static void pwm_lock_all(void)
{
    for(int i = 0; i < sizeof(pwm_out_array) / sizeof(pwm_out_array[0]); i++)
    {
        pwm_out_t *pwm_out = pwm_out_array + i;
        pwm_lock(pwm_out);
    }
}

static void pwm_unlock_all(void)
{
    for(int i = 0; i < sizeof(pwm_out_array) / sizeof(pwm_out_array[0]); i++)
    {
        pwm_out_t *pwm_out = pwm_out_array + i;
        pwm_unlock(pwm_out);
    }
}

static void pwm_lock(pwm_out_t *pwm_out)
{
    GPIOPinTypeGPIOOutput(pwm_out->gpio_port, pwm_out->gpio_pin);
    GPIOPinWrite(pwm_out->gpio_port, pwm_out->gpio_pin, 0x0);
}

static void pwm_unlock(pwm_out_t *pwm_out)
{
    GPIOPinConfigure(pwm_out->gpio_pin_cfg);
    GPIOPinTypePWM(pwm_out->gpio_port, pwm_out->gpio_pin);
}

static void check_timer_entry(void *args)
{
    bool cur_lock = check_lock();
    if(cur_lock == is_locked)
    {
        return;
    }

    is_locked = cur_lock;
    if(is_locked)
    {
        pwm_lock_all();
    }
    else
    {
        pwm_unlock_all();
    }
}

static bool check_lock(void)
{
    return rt_hw_is_error() || !dev_safety_switch_is_off();
}
