/*
 * dev_watchdog.c
 *
 *  Created on: 2019年6月18日
 *      Author: Administrator
 */

#include "dev_watchdog.h"

#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <driverlib/watchdog.h>

#include <msh.h>

/*----------------------------- Private define -------------------------------*/

/*
 * WATCHDOG_SLOW_THREAD_TIMEOUT > WATCHDOG_FAST_THREAD_TIMEOUT
 */
#define WATCHDOG_FAST_THREAD_TIMEOUT  2000     /* ms */
#define WATCHDOG_SLOW_THREAD_TIMEOUT  10000    /* ms */
#define WATCHDOG_THREAD_PRIORITY      (RT_THREAD_PRIORITY_MAX -  177)   /* just below I2C bus thread priority */

/*----------------------------- Private typedef ------------------------------*/



/*------------------------------ Private variables ---------------------------*/

static uint32_t wdt_load_value = 0;
static rt_tick_t last_idle_tick = 0;

/* ----------------------- Private function prototypes ---------------------*/

static rt_err_t wdg_init_hw(void);
static rt_err_t wdt_init_data(void);
static rt_err_t wdt_start(void);
static void wdt_thread_entry(void *arg);
static void wdt_idle_hook(void);

/*-------------------------------- Functions -------------------------------*/

int dev_watchdog_init(void)
{
    wdt_init_data();
    wdg_init_hw();
    wdt_start();

    return 0;
}
INIT_DEVICE_EXPORT(dev_watchdog_init);

/*-------------------------------- Private  Functions -------------------------------*/

static rt_err_t wdg_init_hw(void)
{
    //
    // Enable the Watchdog 0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG0));
    //
    // Check to see if the registers are locked, and if so, unlock them.
    //
    if(WatchdogLockState(WATCHDOG0_BASE) == true)
    {
        WatchdogUnlock(WATCHDOG0_BASE);
    }
    WatchdogReloadSet(WATCHDOG0_BASE, wdt_load_value);
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogStallEnable(WATCHDOG0_BASE);
    //
    // Enable the watchdog timer.
    //
    WatchdogEnable(WATCHDOG0_BASE);

    return RT_EOK;
}

static rt_err_t wdt_init_data(void)
{
    uint32_t timeout_max = UINT32_MAX / SystemCoreClock * 1000;
    /*
     * 第一次超时产生中断，第二次才重启，所以这里除以2
     */
    uint32_t timeout_fast = WATCHDOG_FAST_THREAD_TIMEOUT / 2;
    if(timeout_fast > timeout_max)
    {
        timeout_fast = timeout_max;
    }
    wdt_load_value = timeout_fast * 1000 * (SystemCoreClock / 1000000);
    last_idle_tick = rt_tick_get();

    return RT_EOK;
}

static rt_err_t wdt_start(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("iwg", wdt_thread_entry, RT_NULL,
            512, WATCHDOG_THREAD_PRIORITY, 10);

    if(!tid)
    {
        rt_kprintf("Can't create iwg thread!\n");
        return -RT_ERROR;
    }
    rt_thread_startup(tid);

    rt_thread_idle_sethook(wdt_idle_hook);

    return RT_EOK;
}

static void wdt_thread_entry(void *arg)
{
    int slow_thread_timeout = rt_tick_from_millisecond(WATCHDOG_SLOW_THREAD_TIMEOUT - WATCHDOG_FAST_THREAD_TIMEOUT);

    while(1)
    {
        WatchdogReloadSet(WATCHDOG0_BASE, wdt_load_value);
        if(rt_tick_get() - last_idle_tick >= slow_thread_timeout)
        {
            /* 让看门狗超时从而触发重启 */
            break;
        }

        rt_thread_delay(10);
    }
}

static void wdt_idle_hook(void)
{
    last_idle_tick = rt_tick_get();
}

static void wdt_suicide(int argc, char**argv)
{
    while(1);
}
MSH_CMD_EXPORT(wdt_suicide, wdt suicide);
