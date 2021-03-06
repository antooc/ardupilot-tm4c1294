#ifndef __AP_HAL_Empty_Semaphores_h__
#define __AP_HAL_Empty_Semaphores_h__


#include "AP_HAL_Empty.h"

class Empty::Semaphore : public AP_HAL::Semaphore {
public:
    Semaphore() : _taken(false) {}
    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
private:
    bool _taken;
};

#endif /* __AP_HAL_Empty_Semaphores_h__ */