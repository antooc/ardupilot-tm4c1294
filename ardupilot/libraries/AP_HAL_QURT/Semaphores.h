#ifndef __AP_HAL_QURT_Semaphores_h__
#define __AP_HAL_QURT_Semaphores_h__


#include <AP_HAL/AP_HAL_Boards.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_QURT
#include "AP_HAL_QURT.h"
#include <pthread.h>

class QURT::Semaphore : public AP_HAL::Semaphore {
public:
    Semaphore() {
        pthread_mutex_init(&_lock, nullptr);
    }
    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
private:
    pthread_mutex_t _lock;
};
#endif // CONFIG_HAL_BOARD

#endif /* __AP_HAL_QURT_Semaphores_h__ */