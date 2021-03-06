#ifndef __AP_InertialSensor_AP_InertialSensor_HIL_h__
#define __AP_InertialSensor_AP_InertialSensor_HIL_h__


#include "AP_InertialSensor.h"
#include "AP_InertialSensor_Backend.h"

class AP_InertialSensor_HIL : public AP_InertialSensor_Backend
{
public:
    AP_InertialSensor_HIL(AP_InertialSensor &imu);

    /* update accel and gyro state */
    bool update();

    // detect the sensor
    static AP_InertialSensor_Backend *detect(AP_InertialSensor &imu);

private:
    bool _init_sensor(void);
};

#endif /* __AP_InertialSensor_AP_InertialSensor_HIL_h__ */