/*
 * drv_hrt.c
 *
 *  Created on: 2019Äê1ÔÂ1ÈÕ
 *      Author: Administrator
 */

#include "drv_hrt.h"
#include "hr_timer.h"

hrt_abstime hrt_absolute_time(void)
{
	return (hrt_abstime)hrtimer_get_tick();
}

void hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg)
{
	if(entry->timer.parent.flag & RT_TIMER_FLAG_ACTIVATED)
	{
		hrtimer_stop(&entry->timer);
	}

    hrtimer_init(&entry->timer, "hrt", callout, arg, (rt_tick_t)delay, RT_TIMER_FLAG_ONE_SHOT);
    hrtimer_start(&entry->timer);
}

void hrt_cancel(struct hrt_call *entry)
{
    if(entry->timer.parent.flag & RT_TIMER_FLAG_ACTIVATED)
    {
        hrtimer_stop(&entry->timer);
        rt_memset(entry, 0, sizeof(*entry));
    }
}
