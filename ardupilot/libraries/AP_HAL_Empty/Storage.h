#ifndef __AP_HAL_Empty_Storage_h__
#define __AP_HAL_Empty_Storage_h__


#include "AP_HAL_Empty.h"

class Empty::Storage : public AP_HAL::Storage {
public:
    Storage();
    void init();
    void read_block(void *dst, uint16_t src, size_t n);
    void write_block(uint16_t dst, const void* src, size_t n);
};

#endif /* __AP_HAL_Empty_Storage_h__ */