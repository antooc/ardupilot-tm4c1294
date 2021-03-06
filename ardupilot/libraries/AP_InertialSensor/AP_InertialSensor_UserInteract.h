#ifndef __AP_InertialSensor_AP_InertialSensor_UserInteract_h__
#define __AP_InertialSensor_AP_InertialSensor_UserInteract_h__


#include <AP_Common/AP_Common.h>

/* Pure virtual interface class */
class AP_InertialSensor_UserInteract {
public:
    virtual bool blocking_read() = 0;
    virtual void printf(const char *, ...) FMT_PRINTF(2, 3) = 0;
};

#endif /* __AP_InertialSensor_AP_InertialSensor_UserInteract_h__ */