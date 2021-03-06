/*
 * dev_rc_out.h
 *
 *  Created on: 2019Äê5ÔÂ10ÈÕ
 *      Author: Administrator
 */

#ifndef DEV_RC_OUT_H_
#define DEV_RC_OUT_H_

#include <rtthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RC_OUT_CH_NUM               4
#define PWM_IGNORE_THIS_CHANNEL     UINT16_MAX

/**
 * set all period(all period must be the same)
 *
 * period in microseconds
 */
extern void dev_rc_out_set_period(rt_uint16_t period);


/**
 * set the pulse of each channel
 *
 * @param pulses    an array of pulses, in microseconds
 * @param len       size of the array
 *
 */
extern void dev_rc_out_write(rt_uint16_t *pulses, rt_size_t len);


#ifdef __cplusplus
}
#endif

#endif /* DEV_RC_OUT_H_ */
