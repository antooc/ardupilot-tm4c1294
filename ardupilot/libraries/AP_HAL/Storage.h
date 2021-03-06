#ifndef __AP_HAL_Storage_h__
#define __AP_HAL_Storage_h__


#include <stdint.h>
#include "AP_HAL_Namespace.h"

class AP_HAL::Storage {
public:
    virtual void init() = 0;
    virtual void read_block(void *dst, uint16_t src, size_t n) = 0;
    virtual void write_block(uint16_t dst, const void* src, size_t n) = 0;
};

#endif /* __AP_HAL_Storage_h__ */