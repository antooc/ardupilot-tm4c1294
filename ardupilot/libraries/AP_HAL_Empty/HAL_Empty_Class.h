#ifndef __AP_HAL_Empty_HAL_Empty_Class_h__
#define __AP_HAL_Empty_HAL_Empty_Class_h__


#include <AP_HAL/AP_HAL.h>

#include "AP_HAL_Empty_Namespace.h"
#include "PrivateMember.h"

class HAL_Empty : public AP_HAL::HAL {
public:
    HAL_Empty();
    void run(int argc, char* const* argv, Callbacks* callbacks) const override;
private:
    Empty::PrivateMember *_member;
};

#endif /* __AP_HAL_Empty_HAL_Empty_Class_h__ */