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
#ifndef __AP_Airspeed_AP_Airspeed_MS5525_h__
#define __AP_Airspeed_AP_Airspeed_MS5525_h__


/*
  backend driver for airspeed from I2C
 */

#include <AP_HAL/AP_HAL.h>
#include <AP_Param/AP_Param.h>
#include <AP_HAL/utility/OwnPtr.h>
#include <AP_HAL/I2CDevice.h>
#include <utility>

#include "AP_Airspeed_Backend.h"
#include <AP_HAL/I2CDevice.h>

class AP_Airspeed_MS5525 : public AP_Airspeed_Backend
{
public:
    AP_Airspeed_MS5525(AP_Airspeed &frontend);
    ~AP_Airspeed_MS5525(void) {}
    
    // probe and initialise the sensor
    bool init() override;

    // return the current differential_pressure in Pascal
    bool get_differential_pressure(float &pressure) override;

    // return the current temperature in degrees C, if available
    bool get_temperature(float &temperature) override;

private:
    void measure();
    void collect();
    void timer();
    bool read_prom(void);
    uint16_t crc4_prom(void);
    int32_t read_adc();
    void calculate();

    float pressure;
    float temperature;
    float temperature_sum;
    float pressure_sum;
    uint32_t temp_count;
    uint32_t press_count;
    
    uint32_t last_sample_time_ms;

    uint16_t prom[8];
    uint8_t state;
    int32_t D1;
    int32_t D2;
    
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> dev;
};

#endif /* __AP_Airspeed_AP_Airspeed_MS5525_h__ */