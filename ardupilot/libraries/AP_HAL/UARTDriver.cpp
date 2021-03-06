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

#include "AP_HAL.h"

#include "utility/print_vprintf.h"
#include "UARTDriver.h"

/* 
   BetterStream method implementations
   These are implemented in AP_HAL to ensure consistent behaviour on
   all boards, although they can be overridden by a port
 */

void AP_HAL::UARTDriver::printf(const char *fmt, ...) 
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

void AP_HAL::UARTDriver::vprintf(const char *fmt, va_list ap) 
{
    print_vprintf(this, fmt, ap);
}

void AP_HAL::UARTDriver::printArray(uint8_t *buf, int len, const char *desc)
{
    printf("%s [", desc);
    for(int i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
    }
    printf("]\r\n");
}

void AP_HAL::UARTDriver::logWave(void *buf, int len)
{
    write(0x03);
    write(0xFC);
    write((uint8_t *)buf, len);
    write(0xFC);
    write(0x03);
}
