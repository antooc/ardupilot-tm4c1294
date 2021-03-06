#ifndef __AP_LeakDetector_AP_LeakDetector_Analog_h__
#define __AP_LeakDetector_AP_LeakDetector_Analog_h__


#include "AP_LeakDetector_Backend.h"
#include <AP_HAL/AP_HAL.h>

class AP_LeakDetector_Analog : public AP_LeakDetector_Backend {
public:
    AP_LeakDetector_Analog(AP_LeakDetector &_leak_detector, AP_LeakDetector::LeakDetector_State &_state);
    void read(void);

private:
    AP_HAL::AnalogSource *source;
};

#endif /* __AP_LeakDetector_AP_LeakDetector_Analog_h__ */