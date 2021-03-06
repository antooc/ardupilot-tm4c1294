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
#ifndef __AP_BattMonitor_AP_BattMonitor_Backend_h__
#define __AP_BattMonitor_AP_BattMonitor_Backend_h__


#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include "AP_BattMonitor.h"

class AP_BattMonitor_Backend
{
public:
    // constructor. This incorporates initialisation as well.
    AP_BattMonitor_Backend(AP_BattMonitor &mon, AP_BattMonitor::BattMonitor_State &mon_state);

    // we declare a virtual destructor so that BattMonitor driver can
    // override with a custom destructor if need be
    virtual ~AP_BattMonitor_Backend(void) {}

    // initialise
    virtual void init() {}

    // read the latest battery voltage
    virtual void read() = 0;

    /// capacity_remaining_pct - returns the % battery capacity remaining (0 ~ 100)
    uint8_t capacity_remaining_pct() const;

    /// get capacity for this instance
    int32_t get_capacity() const;

protected:
    AP_BattMonitor                      &_mon;      // reference to front-end
    AP_BattMonitor::BattMonitor_State   &_state;    // reference to this instances state (held in the front-end)
};

#endif /* __AP_BattMonitor_AP_BattMonitor_Backend_h__ */