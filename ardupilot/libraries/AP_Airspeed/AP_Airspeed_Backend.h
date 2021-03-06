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
#ifndef __AP_Airspeed_AP_Airspeed_Backend_h__
#define __AP_Airspeed_AP_Airspeed_Backend_h__


/*
  backend driver class for airspeed
 */

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>

class AP_Airspeed;

class AP_Airspeed_Backend {
public:
    AP_Airspeed_Backend(AP_Airspeed &frontend);
    virtual ~AP_Airspeed_Backend();
    
    // probe and initialise the sensor
    virtual bool init(void) = 0;

    // return the current differential_pressure in Pascal
    virtual bool get_differential_pressure(float &pressure) = 0;

    // return the current temperature in degrees C, if available
    virtual bool get_temperature(float &temperature) = 0;

protected:
    int8_t get_pin(void) const;
    float get_psi_range(void) const;
    uint8_t get_bus(void) const;

    // semaphore for access to shared frontend data
    AP_HAL::Semaphore *sem;    
    
private:
    AP_Airspeed &frontend;
};

#endif /* __AP_Airspeed_AP_Airspeed_Backend_h__ */