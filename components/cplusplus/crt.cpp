/*
* File      : crt.cpp
* This file is part of Device File System in RT-Thread RTOS
* COPYRIGHT (C) 2008-2015, RT-Thread Development Team
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* Change Logs:
* Date           Author       Notes
* 2015-03-07     Bernard      Add copyright header.
*/

#include <rtthread.h>
#include "crt.h"

/*
  globally override new and delete to ensure that we always start with
  zero memory. This ensures consistent behaviour.
 */

void *operator new(size_t size)
{
	if(size < 1)
	{
		size = 1;
	}

	return rt_calloc(size, 1);
}

void *operator new[](size_t size)
{
	if(size < 1)
	{
		size = 1;
	}

	return rt_calloc(size, 1);
}

void operator delete(void *ptr)
{
	if(ptr)
	{
		rt_free(ptr);
	}
}

void operator delete[] (void *ptr)
{
    if(ptr)
    {
    	rt_free(ptr);
    }
}

void __cxa_pure_virtual(void)
{
    rt_kprintf("Illegal to call a pure virtual function.\n");
}
