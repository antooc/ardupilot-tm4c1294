#ifndef __AP_RangeFinder_AP_RangeFinder_MaxsonarSerialLV_h__
#define __AP_RangeFinder_AP_RangeFinder_MaxsonarSerialLV_h__


#include "RangeFinder.h"
#include "RangeFinder_Backend.h"

class AP_RangeFinder_MaxsonarSerialLV : public AP_RangeFinder_Backend
{

public:
    // constructor
    AP_RangeFinder_MaxsonarSerialLV(RangeFinder &ranger, uint8_t instance, RangeFinder::RangeFinder_State &_state,
                                   AP_SerialManager &serial_manager);

    // static detection function
    static bool detect(RangeFinder &ranger, uint8_t instance, AP_SerialManager &serial_manager);

    // update state
    void update(void);

private:
    // get a reading
    bool get_reading(uint16_t &reading_cm);

    AP_HAL::UARTDriver *uart = nullptr;
    uint32_t last_reading_ms = 0;
    char linebuf[10];
    uint8_t linebuf_len = 0;
};

#endif /* __AP_RangeFinder_AP_RangeFinder_MaxsonarSerialLV_h__ */