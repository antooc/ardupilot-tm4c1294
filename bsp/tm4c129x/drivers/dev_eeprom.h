/*
 * dev_eeprom.h
 *
 *  Created on: 2019Äê5ÔÂ9ÈÕ
 *      Author: Administrator
 */

#ifndef DEV_EEPROM_H_
#define DEV_EEPROM_H_

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * src and n a multiple of 4
 */
extern int dev_eeprom_read(rt_uint32_t *dst, rt_uint32_t src, rt_uint32_t n);
extern int dev_eeprom_write(rt_uint32_t dst, rt_uint32_t *src, rt_uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* DEV_EEPROM_H_ */
