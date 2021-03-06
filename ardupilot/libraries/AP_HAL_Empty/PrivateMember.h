#ifndef __AP_HAL_Empty_PrivateMember_h__
#define __AP_HAL_Empty_PrivateMember_h__


/* Just a stub as an example of how to implement a private member of an
 * AP_HAL module */

#include "AP_HAL_Empty.h"

class Empty::PrivateMember {
public:
    PrivateMember(uint16_t foo);
    void init();
private:
    uint16_t _foo;
};

#endif /* __AP_HAL_Empty_PrivateMember_h__ */