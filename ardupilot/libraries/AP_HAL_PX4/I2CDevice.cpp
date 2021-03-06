/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "I2CDevice.h"

#include <AP_HAL/AP_HAL.h>

#include "Util.h"
#include "Scheduler.h"

#include <rtthread.h>
#include <nuttx/board_config.h>
#include <stdio.h>

namespace PX4 {

DeviceBus I2CDevice::businfo[I2CDevice::num_buses];


/*
  map ArduPilot bus numbers to PX4 bus numbers
 */
uint8_t I2CDevice::map_bus_number(uint8_t bus)
{
    switch (bus) {
    case 0:
        // map to internal bus
#ifdef PX4_I2C_BUS_ONBOARD
        return PX4_I2C_BUS_ONBOARD;
#else
        return 0;
#endif

    case 1:
        // map to expansion bus
#ifdef PX4_I2C_BUS_EXPANSION
        return PX4_I2C_BUS_EXPANSION;
#else
        return 0;
#endif
    case 2:
        // map to expansion bus 2
#ifdef PX4_I2C_BUS_EXPANSION1
        return PX4_I2C_BUS_EXPANSION1;
#else
        return 0;
#endif
    }
    // default to bus 0
    return 0;
}

I2CDevice::I2CDevice(uint8_t bus, uint8_t address) :
    _busnum(bus),
    /*_px4dev(_busnum),*/
    _address(address)
{
    set_device_bus(bus);
    set_device_address(address);
    snprintf(_busname, sizeof(_busname), "i2c%d", map_bus_number(bus));
    snprintf(_devname, sizeof(_devname), "I2C:%u:%02x",
             (unsigned)bus, (unsigned)address);
    perf = perf_alloc(PC_ELAPSED, _devname);

    _i2cdev = (struct rt_i2c_bus_device *)rt_device_find(_busname);
    if(!_i2cdev)
    {
        rt_kprintf("i2c device %s not found!\n", _busname);
        return;
    }

    rt_kprintf("I2C device bus %u address 0x%02x opened\n",
               (unsigned)_busnum, (unsigned)_address);
}
    
I2CDevice::~I2CDevice()
{
    rt_kprintf("I2C device bus %u address 0x%02x closed\n",
           (unsigned)_busnum, (unsigned)_address);
    perf_free(perf);
}

bool I2CDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    if(!_i2cdev)
    {
        return false;
    }

    if(_split_transfers || !(send && send_len && recv && recv_len))
    {
        if(send && send_len)
        {
            struct rt_i2c_msg msg;
            rt_size_t ret;

            msg.addr = _address;
            msg.flags = RT_I2C_WR;
            msg.buf = (rt_uint8_t *)send;
            msg.len = send_len;

            ret = rt_i2c_transfer(_i2cdev, &msg, 1);
            if(ret != 1)
            {
                return false;
            }
        }

        if(recv && recv_len)
        {
            struct rt_i2c_msg msg;
            rt_size_t ret;

            msg.addr = _address;
            msg.flags = RT_I2C_RD;
            msg.buf = (rt_uint8_t *)recv;
            msg.len = recv_len;

            ret = rt_i2c_transfer(_i2cdev, &msg, 1);
            if(ret != 1)
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        struct rt_i2c_msg msgs[2];
        rt_size_t ret;

        msgs[0].addr = msgs[1].addr = _address;
        msgs[0].flags = RT_I2C_WR;
        msgs[0].buf = (rt_uint8_t *)send;
        msgs[0].len = send_len;
        msgs[1].flags = RT_I2C_RD;
        msgs[1].buf = (rt_uint8_t *)recv;
        msgs[1].len = recv_len;

        ret = rt_i2c_transfer(_i2cdev,
                    msgs, sizeof(msgs) / sizeof(msgs[0]));

        return ret == sizeof(msgs) / sizeof(msgs[0]);
    }
}

bool I2CDevice::read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                                        uint32_t recv_len, uint8_t times)
{
    return false;
}

    
/*
  register a periodic callback
*/
AP_HAL::Device::PeriodicHandle I2CDevice::register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    if (_busnum >= num_buses) {
        return nullptr;
    }
    struct DeviceBus &binfo = businfo[_busnum];
    return binfo.register_periodic_callback(period_usec, cb, this);
}
    

/*
  adjust a periodic callback
*/
bool I2CDevice::adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    if (_busnum >= num_buses) {
        return false;
    }

    struct DeviceBus &binfo = businfo[_busnum];

    return binfo.adjust_timer(h, period_usec);
}

AP_HAL::OwnPtr<AP_HAL::I2CDevice>
I2CDeviceManager::get_device(uint8_t bus, uint8_t address)
{
    auto dev = AP_HAL::OwnPtr<AP_HAL::I2CDevice>(new I2CDevice(bus, address));
    return dev;
}

}
