/*
 * dev_safty_switch.h
 *
 *  Created on: 2019Äê6ÔÂ3ÈÕ
 *      Author: Administrator
 */

#ifndef DEV_SAFTY_SWITCH_H_
#define DEV_SAFTY_SWITCH_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------ Exported define ------------------------------*/



/*----------------------------- Exported typedef ------------------------------*/



/*-------------------------- Exported variables prototypes -------------------*/



/*--------------------------- Exported function prototypes ------------------*/

rt_bool_t dev_safety_switch_is_off(void);
void dev_safety_switch_set_ready(void);


#ifdef __cplusplus
}
#endif

#endif /* DEV_SAFTY_SWITCH_H_ */
