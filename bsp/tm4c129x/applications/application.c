/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2014, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-07-18     ArdaFu       the first version for TM4C129X
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <drv_hrtimer.h>
#include <hr_timer.h>

#include "gpio_test.h"
#include "hrtimer_test.h"
#include "rc_in_test.h"
#include "i2c_test.h"
#include "dev_safety_switch.h"
#include "board.h"

#ifdef __PX4_NUTTX

int  ArduPilot_main(int argc, char* const argv[]);

static void run_ardupilot(void)
{
    char *argv[] =
    {
            SKETCHNAME,
            "start",
    };

    ArduPilot_main(2, argv);
}


#else

static void run_ardupilot(void)
{
    /* user app entry */
    dev_safety_switch_set_ready();
}

#endif

int main(void)
{
    if(rt_hw_is_error())
    {
        return -1;
    }

    /* user app entry */
    run_ardupilot();
    return 0;
}
