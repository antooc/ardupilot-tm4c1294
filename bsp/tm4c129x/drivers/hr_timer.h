#ifndef __HR_TIMER_H__
#define __HR_TIMER_H__

#include <rtthread.h>
#include <stdint.h>

typedef uint64_t hr_tick_t;
#define HR_TICK_MAX	UINT64_MAX

/**
 * timer structure
 */
struct hrtimer
{
    struct rt_object parent;                            /**< inherit from rt_object */

    rt_list_t        row[RT_TIMER_SKIP_LIST_LEVEL];

    void (*timeout_func)(void *parameter);              /**< timeout function */
    void            *parameter;                         /**< timeout function's parameter */

    hr_tick_t        init_tick;                         /**< timer timeout tick */
    hr_tick_t        timeout_tick;                      /**< timeout tick */
};
typedef struct hrtimer *hrtimer_t;

struct hrtimer_ops
{
	hr_tick_t (*get_cur_tick)(void);
	void (*set_timeout)(hr_tick_t tick);
};
typedef struct hrtimer_ops *hrtimer_ops_t;

void hrtimer_init(hrtimer_t  timer,
                   const char *name,
                   void (*timeout)(void *parameter),
                   void       *parameter,
                   hr_tick_t   time,
                   rt_uint8_t  flag);
rt_err_t hrtimer_detach(hrtimer_t timer);
hrtimer_t hrtimer_create(const char *name,
                           void (*timeout)(void *parameter),
                           void       *parameter,
                           hr_tick_t   time,
                           rt_uint8_t  flag);
rt_err_t hrtimer_delete(hrtimer_t timer);
rt_err_t hrtimer_start(hrtimer_t timer);
rt_err_t hrtimer_stop(hrtimer_t timer);
rt_err_t hrtimer_control(hrtimer_t timer, int cmd, void *arg);
void hrtimer_check(void);
hr_tick_t hrtimer_next_timeout_tick(void);

void system_hrtimer_init(hrtimer_ops_t ops);

hr_tick_t hrtimer_get_tick(void);
void hrtimer_delay(uint32_t us);

#endif /* __HR_TIMER_H__ */
