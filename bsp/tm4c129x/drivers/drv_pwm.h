/*
 * drv_pwm.h
 *
 *  Created on: 2019Äê8ÔÂ2ÈÕ
 *      Author: Administrator
 */

#ifndef DRV_PWM_H_
#define DRV_PWM_H_



#ifdef __cplusplus
extern "C" {
#endif

#include "rtthread.h"

/*------------------------------ Exported define ------------------------------*/



/*----------------------------- Exported typedef ------------------------------*/



/*-------------------------- Exported variables prototypes -------------------*/



/*--------------------------- Exported function prototypes ------------------*/

void drv_pwm_set_frequency(int index, rt_uint32_t frequency);

#ifdef __cplusplus
}
#endif

#endif /* DRV_PWM_H_ */
