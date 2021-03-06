/*
 * i2c_test.c
 *
 *  Created on: 2019Äê6ÔÂ13ÈÕ
 *      Author: Administrator
 */

#include <rtthread.h>
#include <rtdevice.h>

extern rt_err_t i2c_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len);

#if 1
static void write_register(struct rt_i2c_bus_device * _i2cdev, rt_uint8_t _address, rt_uint8_t reg, rt_uint8_t val)
{
    rt_uint8_t buf[2];
    struct rt_i2c_msg msg;
    rt_size_t ret;

    buf[0] = reg;
    buf[1] = val;

    msg.addr = _address;
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = sizeof(buf);

    ret = rt_i2c_transfer(_i2cdev, &msg, 1);
    rt_kprintf("write register, addr:0x%x, reg:0x%x, val:0x%x, ret:%d\n", _address, reg, val, ret);
}
#else

static void write_register(struct rt_i2c_bus_device * _i2cdev, rt_uint8_t _address, rt_uint8_t reg, rt_uint8_t val)
{
    rt_err_t err;

    err = i2c_write_reg(_i2cdev, _address, reg, &val, 1);
    rt_kprintf("write register, addr:0x%x, reg:0x%x, val:0x%x, ret:%s\n", _address, reg, val, err == RT_EOK ? "ok" : "fail");
}

#endif

static void i2c_led_test(void);

void i2c_test(void)
{
    i2c_led_test();
}

#define TOSHIBA_LED_I2C_ADDR 0x55    // default I2C bus address
#define TOSHIBA_LED_I2C_BUS_INTERNAL    0
#define TOSHIBA_LED_I2C_BUS_EXTERNAL    1

#define TOSHIBA_LED_PWM0    0x01    // pwm0 register
#define TOSHIBA_LED_PWM1    0x02    // pwm1 register
#define TOSHIBA_LED_PWM2    0x03    // pwm2 register
#define TOSHIBA_LED_ENABLE  0x04    // enable register

static void i2c_led_test(void)
{
    const char *_busname = "i2c0";
    struct rt_i2c_bus_device * _i2cdev = (struct rt_i2c_bus_device *)rt_device_find(_busname);
    if(!_i2cdev)
    {
        rt_kprintf("i2c device %s not found!\n", _busname);
        return;
    }

    rt_kprintf("i2c device %s opened!\n", _busname);

    write_register(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_ENABLE, 0x03);
    write_register(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_PWM0, 0x0E);
    write_register(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_PWM1, 0x0E);
    write_register(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_PWM2, 0x0E);

//    rt_uint8_t buf[3];
//    buf[0] = buf[1] = buf[2] = 0xff;
//    i2c_write_reg(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_PWM0, buf, 3);

//    rt_uint8_t light = 0;
//    while(1)
//    {
//        rt_uint8_t buf[3];
//        buf[0] = buf[1] = buf[2] = light;
//        buf[2] = 0x0;
//        i2c_write_reg(_i2cdev, TOSHIBA_LED_I2C_ADDR, TOSHIBA_LED_PWM0, buf, 3);
//
//        light++;
//        light %= 0x10;
//        rt_thread_delay(100);
//    }

}
