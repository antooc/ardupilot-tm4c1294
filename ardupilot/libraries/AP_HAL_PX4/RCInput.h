#ifndef __AP_HAL_PX4_RCInput_h__
#define __AP_HAL_PX4_RCInput_h__


#include "AP_HAL_PX4.h"
//#include <nuttx/drv_rc_input.h>
#include <nuttx/perf_counter.h>
#include <pthread.h>
#include "dev_rc_in.h"

#ifndef RC_INPUT_MAX_CHANNELS
#define RC_INPUT_MAX_CHANNELS 18
#endif

class PX4::PX4RCInput : public AP_HAL::RCInput {
public:
    void init() override;
    bool new_input() override;
    uint8_t num_channels() override;
    uint16_t read(uint8_t ch) override;
    uint8_t read(uint16_t* periods, uint8_t len) override;

    bool set_overrides(int16_t *overrides, uint8_t len) override;
    bool set_override(uint8_t channel, int16_t override) override;
    void clear_overrides() override;

    void _timer_tick(void);

    bool rc_bind(int dsmMode) override;

private:
    /* override state */
    uint16_t _override[RC_INPUT_MAX_CHANNELS];
//    struct rc_input_values _rcin;
    rc_in_data_t _rcin;
    int _rc_sub;
    uint64_t _last_read;
    bool _override_valid;
    perf_counter_t _perf_rcin;
    pthread_mutex_t rcin_mutex;

    bool initialized;
};

#endif /* __AP_HAL_PX4_RCInput_h__ */
