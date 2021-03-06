/*
 * drv_hrtimer.h
 *
 *  Created on: 2019Äê3ÔÂ21ÈÕ
 *      Author: Administrator
 */

#ifndef DRV_HRTIMER_H_
#define DRV_HRTIMER_H_

#include <rtthread.h>

#define HRT_PERIOD          0x10000     /* = 65536 */
#define HRT_INTERVAL_MAX    50000
#define HRT_INTERVAL_MIN    50

int drv_hrtimer_init(void);

#endif /* DRV_HRTIMER_H_ */
