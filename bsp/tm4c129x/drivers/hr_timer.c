/*
 * File      : timer.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-03-12     Bernard      first version
 * 2006-04-29     Bernard      implement thread timer
 * 2006-06-04     Bernard      implement hrtimer_control
 * 2006-08-10     Bernard      fix the periodic timer bug
 * 2006-09-03     Bernard      implement hrtimer_detach
 * 2009-11-11     LiJin        add soft timer
 * 2010-05-12     Bernard      fix the timer check bug.
 * 2010-11-02     Charlie      re-implement tick overflow issue
 * 2012-12-15     Bernard      fix the next timeout issue in soft timer
 * 2014-07-12     Bernard      does not lock scheduler when invoking soft-timer
 *                             timeout function.
 */

#include <rtthread.h>
#include <rthw.h>
#include "hr_timer.h"

#define FILE_NAME   "   hr_timer.c"
#define LOG_DBG(fmt, ...) rt_kprintf("[%s:%d] " fmt, FILE_NAME, __LINE__, ##__VA_ARGS__)

/* hard timer list */
static rt_list_t hrtimer_list[RT_TIMER_SKIP_LIST_LEVEL];

static hrtimer_ops_t hrtimer_ops = RT_NULL;

static void _hrtimer_init(hrtimer_t timer,
                           void (*timeout)(void *parameter),
                           void      *parameter,
                           hr_tick_t  time,
                           rt_uint8_t flag)
{
    int i;

    /* set flag */
    timer->parent.flag  = flag;

    /* set deactivated */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;

    timer->timeout_func = timeout;
    timer->parameter    = parameter;

    timer->timeout_tick = 0;
    timer->init_tick    = time;

    /* initialize timer list */
    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        rt_list_init(&(timer->row[i]));
    }
}

/* the fist timer always in the last row */
static hr_tick_t hrtimer_list_next_timeout(rt_list_t timer_list[])
{
    struct hrtimer *timer;

    if (rt_list_isempty(&timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
        return HR_TICK_MAX;

    timer = rt_list_entry(timer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next,
                          struct hrtimer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);

    return timer->timeout_tick;
}

static void _hrtimer_remove(hrtimer_t timer)
{
    int i;

    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        rt_list_remove(&timer->row[i]);
    }
}

//static void idle_timeout(void *parameter)
//{
//    /* do nothing */
////    rt_kprintf("a\n");
//}

#if RT_DEBUG_TIMER
static int hrtimer_count_height(struct hrtimer *timer)
{
    int i, cnt = 0;

    for (i = 0; i < RT_TIMER_SKIP_LIST_LEVEL; i++)
    {
        if (!rt_list_isempty(&timer->row[i]))
            cnt++;
    }
    return cnt;
}

void hrtimer_dump(rt_list_t timer_heads[])
{
    rt_list_t *list;

    for (list = timer_heads[RT_TIMER_SKIP_LIST_LEVEL - 1].next;
         list != &timer_heads[RT_TIMER_SKIP_LIST_LEVEL - 1];
         list = list->next)
    {
        struct hrtimer *timer = rt_list_entry(list,
                                               struct hrtimer,
                                               row[RT_TIMER_SKIP_LIST_LEVEL - 1]);
        rt_kprintf("%d", hrtimer_count_height(timer));
    }
    rt_kprintf("\n");
}
#endif

/**
 * @addtogroup Clock
 */

/**@{*/

/**
 * This function will initialize a timer, normally this function is used to
 * initialize a static timer object.
 *
 * @param timer the static timer object
 * @param name the name of timer
 * @param timeout the timeout function
 * @param parameter the parameter of timeout function
 * @param time the tick of timer
 * @param flag the flag of timer
 */
void hrtimer_init(hrtimer_t  timer,
                   const char *name,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   hr_tick_t   time,
                   rt_uint8_t  flag)
{
    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* timer object initialization */
    //TODO timer object initialization
//    rt_object_init((rt_object_t)timer, RT_Object_Class_Timer, name);
    rt_strncpy(timer->parent.name, name, RT_NAME_MAX);

    _hrtimer_init(timer, timeout, parameter, time, flag);
}
RTM_EXPORT(hrtimer_init);

/**
 * This function will detach a timer from timer management.
 *
 * @param timer the static timer object
 *
 * @return the operation status, RT_EOK on OK; RT_ERROR on error
 */
rt_err_t hrtimer_detach(hrtimer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _hrtimer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    rt_object_detach((rt_object_t)timer);

    return -RT_EOK;
}
RTM_EXPORT(hrtimer_detach);

#ifdef RT_USING_HEAP
/**
 * This function will create a timer
 *
 * @param name the name of timer
 * @param timeout the timeout function
 * @param parameter the parameter of timeout function
 * @param time the tick of timer
 * @param flag the flag of timer
 *
 * @return the created timer object
 */
hrtimer_t hrtimer_create(const char *name,
                           void (*timeout)(void *parameter),
                           void       *parameter,
                           hr_tick_t   time,
                           rt_uint8_t  flag)
{
    struct hrtimer *timer;

    /* allocate a object */
    //TODO hrtimer大小与rt_timer不一样，先临时生成一个，之后再考虑是否加入rt_object管理系统
//    timer = (struct hrtimer *)rt_object_allocate(RT_Object_Class_Timer, name);
    timer = (struct hrtimer *)rt_malloc(sizeof(struct hrtimer));
    rt_strncpy(timer->parent.name, name, RT_NAME_MAX);
    if (timer == RT_NULL)
    {
        return RT_NULL;
    }

    _hrtimer_init(timer, timeout, parameter, time, flag);

    return timer;
}
RTM_EXPORT(hrtimer_create);

/**
 * This function will delete a timer and release timer memory
 *
 * @param timer the timer to be deleted
 *
 * @return the operation status, RT_EOK on OK; RT_ERROR on error
 */
rt_err_t hrtimer_delete(hrtimer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _hrtimer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    rt_object_delete((rt_object_t)timer);

    return -RT_EOK;
}
RTM_EXPORT(hrtimer_delete);
#endif

rt_err_t _hrtimer_start(hrtimer_t timer, rt_bool_t need_schedule)
{
    unsigned int row_lvl;
    rt_list_t *timer_list;
    register rt_base_t level;
    rt_list_t *row_head[RT_TIMER_SKIP_LIST_LEVEL];
    unsigned int tst_nr;
    static unsigned int random_nr;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    /* stop timer firstly */
    level = rt_hw_interrupt_disable();
    /* remove timer from list */
    _hrtimer_remove(timer);
    /* change status of timer */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
    rt_hw_interrupt_enable(level);

//    /*
//     * get timeout tick,
//     * the max timeout tick shall not great than HR_TICK_MAX/2
//     */
//    RT_ASSERT(timer->init_tick < HR_TICK_MAX / 2);
    timer->timeout_tick = hrtimer_ops->get_cur_tick() + timer->init_tick;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    {
        /* insert timer to system timer list */
        timer_list = hrtimer_list;
    }

    row_head[0]  = &timer_list[0];
    for (row_lvl = 0; row_lvl < RT_TIMER_SKIP_LIST_LEVEL; row_lvl++)
    {
        for (; row_head[row_lvl] != timer_list[row_lvl].prev;
             row_head[row_lvl]  = row_head[row_lvl]->next)
        {
            struct hrtimer *t;
            rt_list_t *p = row_head[row_lvl]->next;

            /* fix up the entry pointer */
            t = rt_list_entry(p, struct hrtimer, row[row_lvl]);

            /* If we have two timers that timeout at the same time, it's
             * preferred that the timer inserted early get called early.
             * So insert the new timer to the end the the some-timeout timer
             * list.
             */
            if (t->timeout_tick == timer->timeout_tick)
            {
                continue;
            }
            else if (t->timeout_tick > timer->timeout_tick)
            {
                break;
            }
        }
        if (row_lvl != RT_TIMER_SKIP_LIST_LEVEL - 1)
            row_head[row_lvl + 1] = row_head[row_lvl] + 1;
    }

    /* Interestingly, this super simple timer insert counter works very very
     * well on distributing the list height uniformly. By means of "very very
     * well", I mean it beats the randomness of timer->timeout_tick very easily
     * (actually, the timeout_tick is not random and easy to be attacked). */
    random_nr++;
    tst_nr = random_nr;

    rt_list_insert_after(row_head[RT_TIMER_SKIP_LIST_LEVEL - 1],
                         &(timer->row[RT_TIMER_SKIP_LIST_LEVEL - 1]));
    for (row_lvl = 2; row_lvl <= RT_TIMER_SKIP_LIST_LEVEL; row_lvl++)
    {
        if (!(tst_nr & RT_TIMER_SKIP_LIST_MASK))
            rt_list_insert_after(row_head[RT_TIMER_SKIP_LIST_LEVEL - row_lvl],
                                 &(timer->row[RT_TIMER_SKIP_LIST_LEVEL - row_lvl]));
        else
            break;
        /* Shift over the bits we have tested. Works well with 1 bit and 2
         * bits. */
        tst_nr >>= (RT_TIMER_SKIP_LIST_MASK + 1) >> 1;
    }

    timer->parent.flag |= RT_TIMER_FLAG_ACTIVATED;

    if(need_schedule)
    {
    	hrtimer_ops->set_timeout(hrtimer_next_timeout_tick());
    }

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return -RT_EOK;
}

/**
 * This function will start the timer
 *
 * @param timer the timer to be started
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t hrtimer_start(hrtimer_t timer)
{
	return _hrtimer_start(timer, RT_TRUE);
}
RTM_EXPORT(hrtimer_start);

/**
 * This function will stop the timer
 *
 * @param timer the timer to be stopped
 *
 * @return the operation status, RT_EOK on OK, -RT_ERROR on error
 */
rt_err_t hrtimer_stop(hrtimer_t timer)
{
    register rt_base_t level;

    /* timer check */
    RT_ASSERT(timer != RT_NULL);
    if (!(timer->parent.flag & RT_TIMER_FLAG_ACTIVATED))
        return -RT_ERROR;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    _hrtimer_remove(timer);

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    /* change stat */
    timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;

    return RT_EOK;
}
RTM_EXPORT(hrtimer_stop);

/**
 * This function will get or set some options of the timer
 *
 * @param timer the timer to be get or set
 * @param cmd the control command
 * @param arg the argument
 *
 * @return RT_EOK
 */
rt_err_t hrtimer_control(hrtimer_t timer, int cmd, void *arg)
{
    /* timer check */
    RT_ASSERT(timer != RT_NULL);

    switch (cmd)
    {
    case RT_TIMER_CTRL_GET_TIME:
        *(hr_tick_t *)arg = timer->init_tick;
        break;

    case RT_TIMER_CTRL_SET_TIME:
        timer->init_tick = *(hr_tick_t *)arg;
        break;

    case RT_TIMER_CTRL_SET_ONESHOT:
        timer->parent.flag &= ~RT_TIMER_FLAG_PERIODIC;
        break;

    case RT_TIMER_CTRL_SET_PERIODIC:
        timer->parent.flag |= RT_TIMER_FLAG_PERIODIC;
        break;
    }

    return RT_EOK;
}
RTM_EXPORT(hrtimer_control);

/**
 * This function will check timer list, if a timeout event happens, the
 * corresponding timeout function will be invoked.
 *
 * @note this function shall be invoked in operating system timer interrupt.
 */
void hrtimer_check(void)
{
    struct hrtimer *t;
    hr_tick_t current_tick;
    register rt_base_t level;

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("timer check enter\n"));

    current_tick = hrtimer_ops->get_cur_tick();

    /* disable interrupt */
    level = rt_hw_interrupt_disable();

    while (!rt_list_isempty(&hrtimer_list[RT_TIMER_SKIP_LIST_LEVEL - 1]))
    {
        t = rt_list_entry(hrtimer_list[RT_TIMER_SKIP_LIST_LEVEL - 1].next,
                          struct hrtimer, row[RT_TIMER_SKIP_LIST_LEVEL - 1]);

        /*
         * It supposes that the new tick shall less than the half duration of
         * tick max.
         */
//        rt_kprintf("hrtimer_check, c:%d, t:%d\n", current_tick, t->timeout_tick);
        if (current_tick >= t->timeout_tick)
        {
            /* remove timer from timer list firstly */
            _hrtimer_remove(t);

            /* stop timer */
            t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;

            /* call timeout function */
            t->timeout_func(t->parameter);

            /* re-get tick */
            current_tick = hrtimer_ops->get_cur_tick();

            RT_DEBUG_LOG(RT_DEBUG_TIMER, ("current tick: %d\n", current_tick));

            if ((t->parent.flag & RT_TIMER_FLAG_PERIODIC) &&
                (t->parent.flag & RT_TIMER_FLAG_ACTIVATED))
            {
                /* start it */
                t->parent.flag &= ~RT_TIMER_FLAG_ACTIVATED;
                _hrtimer_start(t, RT_FALSE);
            }

        }
        else
            break;
    }

    hrtimer_ops->set_timeout(hrtimer_next_timeout_tick());

    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    RT_DEBUG_LOG(RT_DEBUG_TIMER, ("timer check leave\n"));
}

/**
 * This function will return the next timeout tick in the system.
 *
 * @return the next timeout tick in the system
 */
hr_tick_t hrtimer_next_timeout_tick(void)
{
    return hrtimer_list_next_timeout(hrtimer_list);
}

/**
 * @ingroup SystemInit
 *
 * This function will initialize system timer
 */
void system_hrtimer_init(hrtimer_ops_t ops)
{
    int i;

    for (i = 0; i < sizeof(hrtimer_list) / sizeof(hrtimer_list[0]); i++)
    {
        rt_list_init(hrtimer_list + i);
    }

    hrtimer_ops = ops;

//    /*
//     * 添加一个超时时间为1000us的周期定时器，以保证就算在没有其他定时任务的情况下也有周期性产生中断，
//     * 从而使得硬件定时器不停运转，以维持高精度时钟计数值。
//     */
//    hrtimer_t idle_timer = hrtimer_create("idle", idle_timeout, RT_NULL, 50000, RT_TIMER_FLAG_PERIODIC);
//    if(!idle_timer)
//    {
//        rt_kprintf("Fail to create idle_timer\n");
//        return;
//    }
//    hrtimer_start(idle_timer);
}

hr_tick_t hrtimer_get_tick(void)
{
	return hrtimer_ops->get_cur_tick();
}

void hrtimer_delay(uint32_t us)
{
    hr_tick_t timeout = hrtimer_get_tick() + us;
    while(hrtimer_get_tick() < timeout);
}

/**@}*/
