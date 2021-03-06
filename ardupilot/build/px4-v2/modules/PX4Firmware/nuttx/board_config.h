/****************************************************************************
 *
 *   Copyright (c) 2013, 2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file board_config.h
 *
 * PX4FMUv2 internal definitions
 */

#ifndef __px4fmu_v2_board_config_h__
#define __px4fmu_v2_board_config_h__

/****************************************************************************************************
 * Included Files
 ****************************************************************************************************/

//#include <px4_config.h>
//#include <nuttx/compiler.h>
#include <stdint.h>

extern "C" {

#define PX4_SPI_BUS_SENSORS	1
#define PX4_SPI_BUS_RAMTRON	2
#define PX4_SPI_BUS_EXT		4
#define PX4_SPI_BUS_BARO	PX4_SPI_BUS_SENSORS

///* Use these in place of the spi_dev_e enumeration to select a specific SPI device on SPI1 */
#define PX4_SPIDEV_GYRO		1
#define PX4_SPIDEV_ACCEL_MAG	2
#define PX4_SPIDEV_BARO		3
#define PX4_SPIDEV_MPU		4
#define PX4_SPIDEV_HMC		5

/* External bus */
#define PX4_SPIDEV_EXT0		1
#define PX4_SPIDEV_EXT1		2
#define PX4_SPIDEV_EXT2		3
#define PX4_SPIDEV_EXT3		4

/* FMUv3 SPI on external bus */
#define PX4_SPIDEV_EXT_MPU		PX4_SPIDEV_EXT0
#define PX4_SPIDEV_EXT_BARO		PX4_SPIDEV_EXT1
#define PX4_SPIDEV_EXT_ACCEL_MAG	PX4_SPIDEV_EXT2
#define PX4_SPIDEV_EXT_GYRO		PX4_SPIDEV_EXT3

/* I2C busses */
//#define PX4_I2C_BUS_EXPANSION	1
#define PX4_I2C_BUS_ONBOARD	    0
#define PX4_I2C_BUS_LED		PX4_I2C_BUS_ONBOARD

/* Devices on the onboard bus.
 *
 * Note that these are unshifted addresses.
 */
#define PX4_I2C_OBDEV_LED	0x55
#define PX4_I2C_OBDEV_HMC5883	0x1e

extern const char *px4_spi_bus_name(int bus_num);

}

#endif /* __px4fmu_v2_board_config_h__ */
