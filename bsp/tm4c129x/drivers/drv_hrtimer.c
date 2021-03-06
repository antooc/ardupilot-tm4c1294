/*
 * drv_hrtimer.c
 *
 *  Created on: 2019Äê3ÔÂ21ÈÕ
 *      Author: Administrator
 */

#include "board.h"
#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#include "drv_hrtimer.h"
#include "hr_timer.h"

#include "driverlib/tiva_timer.h"
#include "inc/hw_timer.h"

/*----------------------------- Private define -------------------------------*/

#define FILE_NAME   "drv_hrtimer.c"
#if 0
#define LOG_DBG(fmt, ...) rt_kprintf("[%s:%d] " fmt, FILE_NAME, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DBG(fmt, ...)
#endif


/*----------------------------- Private typedef ------------------------------*/

/*------------------------------ Private variables ---------------------------*/

/* Timer handler declaration */


/* ----------------------- Private function prototypes ---------------------*/

static rt_err_t timer_init(void);
static rt_err_t timer_start(void);
static hr_tick_t get_cur_tick(void);
static void set_timeout(hr_tick_t tick);

static struct hrtimer_ops ops =
{
    .get_cur_tick = get_cur_tick,
    .set_timeout = set_timeout,
};

/*-------------------------------- Functions -------------------------------*/

int drv_hrtimer_init(void)
{
    rt_err_t err;

    err = timer_init();
    if(err != RT_EOK)
    {
        return -RT_ERROR;
    }

    system_hrtimer_init(&ops);

    err = timer_start();
    if(err != RT_EOK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
INIT_PREV_EXPORT(drv_hrtimer_init);

void Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT | TIMER_TIMA_MATCH);

//    rt_kprintf("ccr:%d\n", HWREG(TIMER0_BASE + TIMER_O_TAR));
    hrtimer_check();
}

/*-------------------------------- Private  Functions -------------------------------*/



static rt_err_t timer_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, SystemCoreClock / 1000000 - 1);
    TimerLoadSet(TIMER0_BASE, TIMER_A, HRT_PERIOD - 1);
    TimerMatchSet(TIMER0_BASE, TIMER_A, HRT_PERIOD - 1 - HRT_INTERVAL_MAX);

    return RT_EOK;
}

static rt_err_t timer_start(void)
{
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_MATCH);
    TimerEnable(TIMER0_BASE, TIMER_A);

    return RT_EOK;
}

static hr_tick_t get_cur_tick(void)
{
    register rt_base_t level;
    hr_tick_t tick;
    register uint32_t  count = 0;

    static volatile hr_tick_t base_tick = 0;
    static volatile uint32_t last_count = 0;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    count = HRT_PERIOD - 1 - HWREG(TIMER0_BASE + TIMER_O_TAR);
    if(count < last_count)
    {
        base_tick += HRT_PERIOD;
    }
    last_count = count;
    tick = base_tick + count;
//    rt_kprintf("get_cur_tick: %d+%d=%d\n", (uint32_t)base_tick, count, (uint32_t)tick);
    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return tick;
}

static void set_timeout(hr_tick_t tick)
{
    register rt_base_t level;
    hr_tick_t cur_tick;
    uint32_t delay;
    uint32_t ccr;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    cur_tick = get_cur_tick();
    if(tick == HR_TICK_MAX)
    {
        tick = cur_tick + HRT_INTERVAL_MAX;
    }

    if(cur_tick < tick)
    {
    	delay = (uint32_t)(tick - cur_tick);
    }
    else
    {
    	delay = 0;
    }
    delay = delay;

    if(tick <= cur_tick + HRT_INTERVAL_MIN)
    {
        tick = cur_tick + HRT_INTERVAL_MIN;
    }

    /* Set the Capture Compare Register value */
    ccr = (uint32_t)((tick) & (HRT_PERIOD - 1));
//    TimerMatchSet(TIMER0_BASE, TIMER_A, ccr);
    HWREG(TIMER0_BASE + TIMER_O_TAMATCHR) = HRT_PERIOD - 1 - ccr;
    LOG_DBG("ct:%d, nt:%d, delay:%d, ccr:%d\n", (uint32_t)cur_tick, (uint32_t)tick, delay, ccr);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);
}

