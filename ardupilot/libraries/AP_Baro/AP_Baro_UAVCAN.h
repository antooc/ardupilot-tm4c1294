#ifndef __AP_Baro_AP_Baro_UAVCAN_h__
#define __AP_Baro_AP_Baro_UAVCAN_h__


#include "AP_Baro_Backend.h"
#include <AP_UAVCAN/AP_UAVCAN.h>

class AP_Baro_UAVCAN : public AP_Baro_Backend {
public:
    AP_Baro_UAVCAN(AP_Baro &);
    ~AP_Baro_UAVCAN() override;

    void update() override;

    // This method is called from UAVCAN thread
    virtual void handle_baro_msg(float pressure, float temperature) override;

private:
    uint8_t _instance;
    float _pressure;
    float _temperature;
    uint64_t _last_timestamp;

    AP_HAL::Semaphore *_sem_baro;
};

#endif /* __AP_Baro_AP_Baro_UAVCAN_h__ */