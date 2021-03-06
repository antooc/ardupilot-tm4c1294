/*
 * drv_i2c.c
 *
 *  Created on: 2019Äê6ÔÂ3ÈÕ
 *      Author: Administrator
 */


#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include "driverlib/i2c.h"
#include "hr_timer.h"

/*----------------------------- Private define -------------------------------*/

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

/*----------------------------- Private typedef ------------------------------*/

typedef struct
{
    struct rt_i2c_bus_device bus_dev;
    char name[RT_NAME_MAX];

    rt_uint32_t gpio_periph;
    rt_uint32_t gpio_port;
    rt_uint32_t gpio_scl_pin;
    rt_uint32_t gpio_scl_cfg;
    rt_uint32_t gpio_sda_pin;
    rt_uint32_t gpio_sda_cfg;

    rt_uint32_t i2c_periph;
    rt_uint32_t i2c_base;
} i2c_device_t;

typedef enum
{
    I2C_START = 0,
    I2C_CONT,
    I2C_STOP,
    I2C_ABORT,
} i2c_cmd_e;


/*------------------------------ Private variables ---------------------------*/

static rt_size_t i2c_master_xfer(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[], rt_uint32_t num);

static struct rt_i2c_bus_device_ops i2c_ops =
{
        .master_xfer = i2c_master_xfer,
        .i2c_bus_control = RT_NULL,
        .slave_xfer = RT_NULL,

};

i2c_device_t i2c_devs[] =
{
        {
                 .bus_dev =
                 {
                         .ops = &i2c_ops,
                         .timeout = 0,
                 },
                .name = "i2c0",

                .gpio_periph = SYSCTL_PERIPH_GPIOB,
                .gpio_port = GPIO_PORTB_BASE,
                .gpio_scl_pin = GPIO_PIN_2,
                .gpio_scl_cfg = GPIO_PB2_I2C0SCL,
                .gpio_sda_pin = GPIO_PIN_3,
                .gpio_sda_cfg = GPIO_PB3_I2C0SDA,

                .i2c_periph = SYSCTL_PERIPH_I2C0,
                .i2c_base = I2C0_BASE,
        },
};

/* ----------------------- Private function prototypes ---------------------*/
static rt_err_t hw_i2c_unit_init(i2c_device_t *unit);

#define I2C_CHECK_ERR(err)     \
{\
    if(err != RT_EOK)\
    {\
        goto ERROR;\
    }\
}

rt_err_t i2c_wait_and_check_err(i2c_device_t *i2c_dev);
static rt_err_t i2c_send_bytes(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg, rt_bool_t is_last_msg);
static rt_err_t i2c_recv_bytes(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg, rt_bool_t is_last_msg);
static rt_err_t i2c_abort(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg);

/*-------------------------------- Functions -------------------------------*/

int rt_hw_i2c_init(void)
{
    rt_err_t err;

    for(int i = 0; i < ARRAY_SIZE(i2c_devs); i++)
    {
        err = hw_i2c_unit_init(i2c_devs + i);
        if(err != RT_EOK)
        {
            rt_kprintf("ii2_unit_init fail, index:%d\n", i);
            return err;
        }
        rt_i2c_bus_device_register(&i2c_devs[i].bus_dev, i2c_devs[i].name);
    }

    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);

rt_err_t i2c_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t slave_addr, rt_uint8_t reg_addr, rt_uint8_t *data, rt_size_t len)
{
    i2c_device_t *i2c_dev = (i2c_device_t *)bus;
    uint32_t i2c_base = i2c_dev->i2c_base;

    //
    // Specify slave address
    //
    I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);
    //
    // Place the character to be sent in the data register
    //
    I2CMasterDataPut(i2c_base, reg_addr);
    //
    // Initiate send of character from Master to Slave
    //
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
    //
    // Delay until transmission completes
    //
    if(i2c_wait_and_check_err(i2c_dev) != RT_EOK)
    {
        goto ERROR;
    }

    for(rt_size_t i = 0; i < len; i++)
    {
        I2CMasterDataPut(i2c_base, data[i]);
        if(i < len - 1)
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        else
        {
            I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        if(i2c_wait_and_check_err(i2c_dev) != RT_EOK)
        {
            goto ERROR;
        }
    }

    return RT_EOK;

ERROR:
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
    while(I2CMasterBusy(i2c_base));
    return -RT_EIO;
}

/*-------------------------------- Private  Functions -------------------------------*/

static rt_err_t hw_i2c_unit_init(i2c_device_t *unit)
{
    /* Output value must be set to 1 to not drive lines low. Set
     SCL first, to ensure it is high before changing SDA. */

    SysCtlPeripheralEnable(unit->gpio_periph);
    while(!SysCtlPeripheralReady(unit->gpio_periph));

//    GPIOPadConfigSet(unit->gpio_port, unit->gpio_scl_pin, GPIO_STRENGTH_2MA, PIN_MODE_OUTPUT_OD);
//    GPIODirModeSet(unit->gpio_port, unit->gpio_scl_pin, GPIO_DIR_MODE_OUT);
//    GPIOPinWrite(unit->gpio_port, unit->gpio_scl_pin, unit->gpio_scl_pin);
//
//    GPIOPadConfigSet(unit->gpio_port, unit->gpio_sda_pin, GPIO_STRENGTH_2MA, PIN_MODE_OUTPUT_OD);
//    GPIODirModeSet(unit->gpio_port, unit->gpio_sda_pin, GPIO_DIR_MODE_OUT);
//    GPIOPinWrite(unit->gpio_port, unit->gpio_sda_pin, unit->gpio_sda_pin);
//
//    /* In some situations, after a reset during an I2C transfer, the slave
//     device may be left in an unknown state. Send 9 clock pulses to
//     set slave in a defined state. */
//    for (int i = 0; i < 9; i++)
//    {
//        GPIOPinWrite(unit->gpio_port, unit->gpio_scl_pin, unit->gpio_scl_pin);
//        GPIOPinWrite(unit->gpio_port, unit->gpio_scl_pin, 0);
//    }

    //
    // Configure gpio
    //
    GPIOPinConfigure(unit->gpio_scl_cfg);
    GPIOPinConfigure(unit->gpio_sda_cfg);
    GPIOPinTypeI2C(unit->gpio_port, unit->gpio_sda_pin);
    GPIOPinTypeI2CSCL(unit->gpio_port, unit->gpio_scl_pin);

    //
    // Stop the Clock, Reset and Enable I2C Module
    // in Master Function
    //
    SysCtlPeripheralDisable(unit->i2c_periph);
    SysCtlPeripheralReset(unit->i2c_periph);
    SysCtlPeripheralEnable(unit->i2c_periph);
    while(!SysCtlPeripheralReady(unit->i2c_periph));

    //
    // Initialize Master and Slave
    //
    I2CMasterInitExpClk(unit->i2c_base, SystemCoreClock, false);
    //
    // Enable i2c
    //
    I2CMasterEnable(unit->i2c_base);

    return RT_EOK;
}

static rt_size_t i2c_master_xfer(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    rt_size_t size_sum = 0;
    i2c_device_t *i2c_dev = (i2c_device_t *)bus;
    struct rt_i2c_msg *msg = RT_NULL;
    rt_err_t err;

    for(int i = 0; i < num; i++)
    {
        msg = msgs + i;

        if (msg->flags & RT_I2C_RD)
        {
            err = i2c_recv_bytes(i2c_dev, msg, i == num - 1);
            I2C_CHECK_ERR(err);
        }
        else
        {
            err = i2c_send_bytes(i2c_devs, msg, i == num - 1);
            I2C_CHECK_ERR(err);
        }
        size_sum++;
    }

    return size_sum;

ERROR:
    i2c_abort(i2c_dev, msg);
    return size_sum;
}

rt_err_t i2c_wait_and_check_err(i2c_device_t *i2c_dev)
{
//    while(!I2CMasterBusy(i2c_dev->i2c_base));
    hrtimer_delay(10);
    while(I2CMasterBusy(i2c_dev->i2c_base));
    uint32_t err = I2CMasterErr(i2c_dev->i2c_base);
    if(err)
    {
//        debug_printf("I2C master error:0x%x", err);
//        RT_ASSERT(!err);

        return -RT_EIO;
    }
    else
    {
        return RT_EOK;
    }
}

static rt_err_t i2c_send_bytes(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg, rt_bool_t is_last_msg)
{
    rt_err_t err;

    for(rt_size_t i = 0; i < msg->len; i++)
    {
        I2CMasterDataPut(i2c_dev->i2c_base, msg->buf[i]);
        if(i == 0)
        {
            if(!(msg->flags & RT_I2C_NO_START))
            {
                I2CMasterSlaveAddrSet(i2c_dev->i2c_base, msg->addr, false);
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
            }
            else
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);
            }
        }
        else if(i < msg->len - 1)
        {
            I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        else
        {
            if(is_last_msg)
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);
            }
            else
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);
            }
        }
        err = i2c_wait_and_check_err(i2c_dev);
        if(err != RT_EOK)
        {
            return err;
        }
    }

    return RT_EOK;
}

static rt_err_t i2c_recv_bytes(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg, rt_bool_t is_last_msg)
{
    rt_err_t err;

    for(rt_size_t i = 0; i < msg->len; i++)
    {
        if(i == 0)
        {
            if(!(msg->flags & RT_I2C_NO_START))
            {
                I2CMasterSlaveAddrSet(i2c_dev->i2c_base, msg->addr, true);
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_START);
            }
            else
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
            }
        }
        else if(i < msg->len - 1)
        {
            I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }
        else
        {
            if(is_last_msg)
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
            }
            else
            {
                I2CMasterControl(i2c_dev->i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
            }
        }
        err = i2c_wait_and_check_err(i2c_dev);
        if(err != RT_EOK)
        {
            return err;
        }

        msg->buf[i] = I2CMasterDataGet(i2c_dev->i2c_base);
    }

    return RT_EOK;
}

static rt_err_t i2c_abort(i2c_device_t *i2c_dev, struct rt_i2c_msg *msg)
{
    I2CMasterControl(i2c_dev->i2c_base, msg && msg->flags & RT_I2C_RD ?
                     I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP : I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
    while(I2CMasterBusy(i2c_dev->i2c_base));

    return RT_EOK;
}






