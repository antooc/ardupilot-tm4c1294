/*
 * dev_rc_in.h
 *
 *  Created on: 2019Äê5ÔÂ10ÈÕ
 *      Author: Administrator
 */

#ifndef DEV_RC_IN_H_
#define DEV_RC_IN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <stdint.h>
#include <stdbool.h>

#define RC_IN_CH_NUM    18

typedef struct
{
//    uint64_t timestamp_publication;
    uint64_t timestamp_last_signal;
    uint32_t channel_count;
//    int32_t rssi;
    bool rc_failsafe;
//    bool rc_lost;
//    uint16_t rc_lost_frame_count;
//    uint16_t rc_total_frame_count;
//    uint16_t rc_ppm_frame_length;
//    uint8_t input_source;
    uint16_t values[RC_IN_CH_NUM];    /* an array of periods, in microseconds */
} rc_in_data_t;

extern void dev_rc_in_read(rc_in_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* DEV_RC_IN_H_ */
