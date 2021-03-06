/*
 * drv_hrt.h
 *
 *  Created on: 2019Äê1ÔÂ1ÈÕ
 *      Author: Administrator
 */

#ifndef DRV_HRT_H_
#define DRV_HRT_H_

#include <stdint.h>
#include <hr_timer.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Absolute time, in microsecond units.
 *
 * Absolute time is measured from some arbitrary epoch shortly after
 * system startup.  It should never wrap or go backwards.
 */
typedef uint64_t	hrt_abstime;

/**
 * Callout function type.
 *
 * Note that callouts run in the timer interrupt context, so
 * they are serialised with respect to each other, and must not
 * block.
 */
typedef void	(* hrt_callout)(void *arg);

/**
 * Callout record.
 */
typedef struct hrt_call {
//	struct sq_entry_s	link;
    struct hrtimer  timer;

//	hrt_abstime		deadline;
//	hrt_abstime		period;
//	hrt_callout		callout;
//	void			*arg;
} *hrt_call_t;

/**
 * Get absolute time.
 */
extern hrt_abstime hrt_absolute_time(void);

/**
 * Call callout(arg) after delay has elapsed.
 *
 * If callout is NULL, this can be used to implement a timeout by testing the call
 * with hrt_called().
 */
extern void	hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg);

extern void hrt_cancel(struct hrt_call *entry);

#ifdef __cplusplus
}
#endif

#endif /* DRV_HRT_H_ */
