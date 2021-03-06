/*
 * misc.h
 *
 *  Created on: 2019Äê3ÔÂ13ÈÕ
 *      Author: Administrator
 */

#ifndef MISC_H_
#define MISC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TEST_LED_NUM	2

extern void px4_systemreset(bool hold_in_bootloader);
extern bool up_interrupt_context(void);

extern void test_led_init(void);
extern void test_led_turn_on(int num);
extern void test_led_turn_off(int num);
extern void test_led_toggle(int num);

#ifdef __cplusplus
}
#endif

#endif /* MISC_H_ */
