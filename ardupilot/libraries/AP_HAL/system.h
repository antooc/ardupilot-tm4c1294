#ifndef __AP_HAL_system_h__
#define __AP_HAL_system_h__


#include <stdint.h>

#include <AP_Common/AP_Common.h>

#include "AP_HAL_Macros.h"

namespace AP_HAL {

void init();

void panic(const char *errormsg, ...) FMT_PRINTF(1, 2) NORETURN;

uint32_t micros();
uint32_t millis();
uint64_t micros64();
uint64_t millis64();

} // namespace AP_HAL

#endif /* __AP_HAL_system_h__ */