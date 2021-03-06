/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
   driver for Cheerson CX-OF optical flow sensor

   LC302 serial packet description
   byte0: header (0xFE)
   byte1: reserved
   byte2: x-motion low byte;
   byte3: x-motion high byte;
   byte4: y-motion low byte;
   byte5: y-motion high byte;
   byte6: t-motion
   byte7: surface quality
   byte8: footer (0xAA)

   sensor sends packets at 25hz
 */

#include <AP_HAL/AP_HAL.h>
#include "AP_OpticalFlow_LC302.h"
#include <AP_Math/crc.h>
#include <AP_AHRS/AP_AHRS.h>
#include <utility>
#include "OpticalFlow.h"
#include <stdio.h>

#define LC302_HEADER         (uint8_t)0xFE
#define LC302_FOOTER         (uint8_t)0x55
//#define LC302_FRAME_LENGTH               9
#define LC302_PIXEL_SCALING      (1e-4)
#define LC302_TIMEOUT_SEC             0.3f

extern const AP_HAL::HAL& hal;

// constructor
AP_OpticalFlow_LC302::AP_OpticalFlow_LC302(OpticalFlow &_frontend, AP_HAL::UARTDriver *_uart) :
    OpticalFlow_backend(_frontend),
    uart(_uart)
{
}

// detect the device
AP_OpticalFlow_LC302 *AP_OpticalFlow_LC302::detect(OpticalFlow &_frontend, AP_SerialManager &serial_manager)
{
    // look for first serial driver with protocol defined as OpticalFlow
    // this is the only optical flow sensor which uses the serial protocol
    AP_HAL::UARTDriver *uart = serial_manager.find_serial(AP_SerialManager::SerialProtocol_OpticalFlow, 0);
    if (uart == nullptr) {
        return nullptr;
    }

    // we have found a serial port so use it
    AP_OpticalFlow_LC302 *sensor = new AP_OpticalFlow_LC302(_frontend, uart);
    return sensor;
}

uint8_t AP_OpticalFlow_LC302::calXor(optical_flow_pkg_t *pkg)
{
    uint8_t _xor = 0;
    for(uint8_t *ptr = (uint8_t *)&pkg->flow_data; ptr < (uint8_t *)&pkg->_xor; ptr++)
    {
        _xor ^= *ptr;
    }
    return _xor;
}

// initialise the sensor
void AP_OpticalFlow_LC302::init()
{
    // sanity check uart
    if (uart == nullptr) {
        return;
    }

    // open serial port with baud rate of 19200
    uart->begin(19200);

    last_frame_us = AP_HAL::micros();
}

// read latest values from sensor and fill in x,y and totals.
void AP_OpticalFlow_LC302::update(void)
{
    // sanity check uart
    if (uart == nullptr) {
        return;
    }

    // record gyro values as long as they are being used
    // the sanity check of dt below ensures old gyro values are not used
    if (gyro_sum_count < 1000) {
        const Vector3f& gyro = get_ahrs().get_gyro();
        gyro_sum.x += gyro.x;
        gyro_sum.y += gyro.y;
        gyro_sum_count++;
    }

    // read any available characters in the serial buffer
    int16_t nbytes = uart->available();
    while (nbytes-- > 0) {
        int16_t r = uart->read();
        if (r < 0) {
            continue;
        }
        uint8_t c = (uint8_t)r;
        // if buffer is empty and this byte is header, add to buffer
        if (buf_len == 0) {
            if (c == LC302_HEADER) {
                buf[buf_len++] = c;
            }
        } else {
            // add character to buffer
            buf[buf_len++] = c;

            // if buffer has 9 items try to decode it
            if (buf_len >= sizeof(buf)) {

                optical_flow_pkg_t *pkg = (optical_flow_pkg_t *)buf;

                // check last character matches footer
                if (pkg->footer != LC302_FOOTER
                        || pkg->fixed_0a != 0x0A
                        || pkg->_xor != calXor(pkg)) {

//                    hal.console->printArray(buf, buf_len, "lc302");
//                    hal.console->printf("xor:%02x\n", calXor(pkg));

                    buf_len = 0;

                    continue;
                }

//                hal.console->printf("x:%d, y:%d, q:%u, dt:%u\r\n",
//                                    pkg->flow_data.flow_x_integral,
//                                    pkg->flow_data.flow_y_integral,
//                                    pkg->flow_data.quality,
//                                    pkg->flow_data.integration_timespan);

                // add to sum of all readings from sensor this iteration
                flow_count++;
                x_sum += pkg->flow_data.flow_x_integral;
                y_sum += pkg->flow_data.flow_y_integral;
                qual_sum += pkg->flow_data.quality;
                timespan_sum += pkg->flow_data.integration_timespan;

                // clear buffer
                buf_len = 0;
            }
        }
    }

    // return without updating state if no readings
    if (flow_count < 3) {
        return;
    }

    struct OpticalFlow::OpticalFlow_state state {};

    // average surface quality scaled to be between 0 and 255
    state.surface_quality = qual_sum / flow_count;

    // calculate dt
//    uint64_t this_frame_us = uart->receive_time_constraint_us(LC302_FRAME_LENGTH);
    uint64_t this_frame_us = 0;
    if (this_frame_us == 0) {
        // for HAL that cannot estimate arrival time in serial buffer use current time
        this_frame_us = AP_HAL::micros();
    }
    float dt = (this_frame_us - last_frame_us) * 1.0e-6;
    last_frame_us = this_frame_us;

    // sanity check dt
    if (is_positive(dt) && (dt < LC302_TIMEOUT_SEC)) {

        // 之前的dt仅仅是用于判断超时，这里使用传感器上报的时间来计算
        dt = timespan_sum / flow_count * 1.0e-6;

        // calculate flow values
        const Vector2f flowScaler = _flowScaler();
        float flowScaleFactorX = 1.0f + 0.001f * flowScaler.x;
        float flowScaleFactorY = 1.0f + 0.001f * flowScaler.y;

        // copy flow rates to state structure
        state.flowRate = Vector2f(((float)x_sum / flow_count) * flowScaleFactorX,
                                  ((float)y_sum / flow_count) * flowScaleFactorY);
        state.flowRate *= LC302_PIXEL_SCALING / dt;

        // copy average body rate to state structure
        state.bodyRate = Vector2f(gyro_sum.x / gyro_sum_count, gyro_sum.y / gyro_sum_count);

        _applyYaw(state.flowRate);
        _applyYaw(state.bodyRate);
    } else {
        // first frame received in some time so cannot calculate flow values
        state.flowRate.zero();
        state.bodyRate.zero();
    }

    _update_frontend(state);

    // reset gyro sum
    gyro_sum.zero();
    gyro_sum_count = 0;

    // reset sensor data
    x_sum = 0;
    y_sum = 0;
    qual_sum = 0;
    timespan_sum = 0;
    flow_count = 0;
}
