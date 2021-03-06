/*
 * drv_spi_sensors.c
 *
 *  Created on: 2019Äê3ÔÂ28ÈÕ
 *      Author: Administrator
 */

#include <rtthread.h>
#include <drv_spi.h>

/*----------------------------- Private define -------------------------------*/

#define SPI_SENSOR_BUS_NAME         "spi2"
#define SPI_LSM9DS0_AM_CS			11		// PP2
#define SPI_LSM9DS0_G_CS			12		// PN3
#define SPI_MPU9250_CS				13		// PN2
#define SPI_MS5611_CS               17      // PH3

#define MPUREG_WHOAMI 0x75
#define MPU_WHOAMI_MPU60X0  0x68
#define MPU_WHOAMI_MPU9250  0x71
#define MPU_WHOAMI_ICM20608 0xaf
#define MPU_WHOAMI_ICM20602 0x12

#define LSMREG_WHOAMI 0x0f
#define LSM_WHOAMI_LSM303D 0x49


/*----------------------------- Private typedef ------------------------------*/



/*------------------------------ Private variables ---------------------------*/



/* ----------------------- Private function prototypes ---------------------*/

static void spi_sensor_test(void);
static rt_err_t spi_check_register(const char *devname, rt_uint8_t regnum, rt_uint8_t value);
static rt_err_t spi_check_register2(const char *devname, rt_uint8_t regnum, rt_uint8_t value);

/*-------------------------------- Functions -------------------------------*/

int rt_spi_sensors_init(void)
{
    hw_spi_bus_attach_device(SPI_LSM9DS0_G_CS, SPI_SENSOR_BUS_NAME, "lsm9ds0_g");
    hw_spi_bus_attach_device(SPI_LSM9DS0_AM_CS, SPI_SENSOR_BUS_NAME, "lsm9ds0_am");
    hw_spi_bus_attach_device(SPI_MPU9250_CS, SPI_SENSOR_BUS_NAME, "mpu9250");
    hw_spi_bus_attach_device(SPI_MS5611_CS, SPI_SENSOR_BUS_NAME, "ms5611");

    spi_sensor_test();

    return RT_EOK;
}
INIT_DEVICE_EXPORT(rt_spi_sensors_init);


/*-------------------------------- Private  Functions -------------------------------*/

__attribute__((unused))
static void spi_sensor_test(void)
{
    spi_check_register2("lsm9ds0_am", LSMREG_WHOAMI, LSM_WHOAMI_LSM303D);
    spi_check_register2("mpu9250", MPUREG_WHOAMI, MPU_WHOAMI_MPU9250);
}

__attribute__((unused))
static rt_err_t spi_check_register(const char *devname, rt_uint8_t regnum, rt_uint8_t value)
{
    struct rt_spi_device *spidev = RT_NULL;
    struct rt_spi_configuration cfg;
    rt_uint8_t read_value = 0;

    spidev = (struct rt_spi_device *)rt_device_find(devname);
    if(spidev == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\n", devname);
        return -RT_ERROR;
    }

    cfg.data_width = 8;
    cfg.mode = RT_SPI_MODE_3;
    cfg.mode |= RT_SPI_MSB;
    cfg.max_hz = 11 * 1000 * 1000;
    rt_spi_configure(spidev, &cfg);

    regnum |= 0x80;

    rt_spi_send_then_recv(spidev, &regnum, 1, &read_value, 1);
    rt_kprintf("[%s:0x%x] expected:0x%x, read:0x%x\n", devname, regnum, value, read_value);

    return value == read_value ? RT_EOK : -RT_ERROR;
}

__attribute__((unused))
static rt_err_t spi_check_register2(const char *devname, rt_uint8_t regnum, rt_uint8_t value)
{
    struct rt_spi_device *spidev = RT_NULL;
    struct rt_spi_configuration cfg;
    rt_uint8_t buf[2];

    spidev = (struct rt_spi_device *)rt_device_find(devname);
    if(spidev == RT_NULL)
    {
        rt_kprintf("spi device %s not found!\n", devname);
        return -RT_ERROR;
    }

    cfg.data_width = 8;
    cfg.mode = RT_SPI_MODE_3;
    cfg.mode |= RT_SPI_MSB;
    cfg.max_hz = 11 * 1000 * 1000;
    rt_spi_configure(spidev, &cfg);

    regnum |= 0x80;
    buf[0] = regnum;
    buf[1] = 0x00;

    rt_spi_transfer(spidev, buf, buf, sizeof(buf) / sizeof(buf[0]));
    rt_kprintf("[%s:0x%x] expected:0x%x, read:0x%x\n", devname, regnum, value, buf[1]);

    return value == buf[1] ? RT_EOK : -RT_ERROR;
}
