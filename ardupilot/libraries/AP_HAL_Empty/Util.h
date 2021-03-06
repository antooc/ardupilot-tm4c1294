#ifndef __AP_HAL_Empty_Util_h__
#define __AP_HAL_Empty_Util_h__


#include <AP_HAL/AP_HAL.h>
#include "AP_HAL_Empty_Namespace.h"

class Empty::Util : public AP_HAL::Util {
public:
    bool run_debug_shell(AP_HAL::BetterStream *stream) { return false; }
};

#endif /* __AP_HAL_Empty_Util_h__ */