#ifndef __AP_OpticalFlow_AP_OpticalFlow_LC302_h__
#define __AP_OpticalFlow_AP_OpticalFlow_LC302_h__


#include "OpticalFlow.h"
#include <AP_HAL/utility/OwnPtr.h>
#include <AP_SerialManager/AP_SerialManager.h>

#pragma pack (1)

typedef struct
{
    int16_t     flow_x_integral;
    int16_t     flow_y_integral;
    uint16_t    integration_timespan;
    uint16_t    ground_distance;
    uint8_t     quality;
    uint8_t     version;
} optical_flow_data_t;

typedef struct
{
    uint8_t header;
    uint8_t fixed_0a;
    optical_flow_data_t flow_data;
    uint8_t _xor;
    uint8_t footer;
} optical_flow_pkg_t;

#pragma pack ()

class AP_OpticalFlow_LC302 : public OpticalFlow_backend
{
public:
    /// constructor
    AP_OpticalFlow_LC302(OpticalFlow &_frontend, AP_HAL::UARTDriver *uart);

    // initialise the sensor
    void init() override;

    // read latest values from sensor and fill in x,y and totals.
    void update(void) override;

    // detect if the sensor is available
    static AP_OpticalFlow_LC302 *detect(OpticalFlow &_frontend, AP_SerialManager &serial_manager);

    static uint8_t calXor(optical_flow_pkg_t *pkg);

private:

    AP_HAL::UARTDriver *uart;           // uart connected to flow sensor
    uint64_t last_frame_us;             // system time of last message from flow sensor
    uint8_t buf[sizeof(optical_flow_pkg_t)];      // buff of characters received from flow sensor
    uint8_t buf_len;                    // number of characters in buffer
    Vector2f gyro_sum;                  // sum of gyro sensor values since last frame from flow sensor
    uint16_t gyro_sum_count;            // number of gyro sensor values in sum

    // sensor values
    int32_t x_sum;
    int32_t y_sum;
    uint16_t qual_sum;
    uint32_t timespan_sum;
    uint16_t flow_count;
};


#endif /* __AP_OpticalFlow_AP_OpticalFlow_LC302_h__ */
