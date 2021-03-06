/*
 * drv_gpio.c
 *
 *  Created on: 2019Äê4ÔÂ9ÈÕ
 *      Author: Administrator
 */

#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include "driverlib/gpio.h"

/*----------------------------- Private define -------------------------------*/

#define __TI_PIN(index, gpio, gpio_index) {index, SYSCTL_PERIPH_GPIO##gpio, GPIO_PORT##gpio##_BASE, GPIO_PIN_##gpio_index}
#define __TI_PIN_DEFAULT(index) {-1, 0, 0, 0}
#define ITEM_NUM(items) sizeof(items) / sizeof(items[0])

/*----------------------------- Private typedef ------------------------------*/

struct pin_index
{
    int index;
    uint32_t gpio_periph;
    uint32_t gpio_port;
    uint32_t gpio_pin;
};

/*------------------------------ Private variables ---------------------------*/

static const struct pin_index pins[] =
{
    __TI_PIN_DEFAULT(0),     /* unused */

    /*
     * BoosterPack1
     */

    __TI_PIN_DEFAULT(1),     /* unused */
    __TI_PIN_DEFAULT(2),     /* unused */
    __TI_PIN_DEFAULT(3),     /* unused */
    __TI_PIN_DEFAULT(4),     /* unused */
    __TI_PIN(5, C, 6),       /* unused */
    __TI_PIN_DEFAULT(6),     /* unused */
    __TI_PIN_DEFAULT(7),     /* unused */
    __TI_PIN(8, C, 7),       /* unused */
    __TI_PIN_DEFAULT(9),     /* unused */
    __TI_PIN_DEFAULT(10),     /* unused */
    __TI_PIN(11, P, 2),       /* unused */
    __TI_PIN(12, N, 3),       /* unused */
    __TI_PIN(13, N, 2),       /* unused */
    __TI_PIN_DEFAULT(14),     /* unused */
    __TI_PIN_DEFAULT(15),     /* unused */
    __TI_PIN_DEFAULT(16),     /* unused */
    __TI_PIN(17, H, 3),       /* unused */
    __TI_PIN(18, H, 2),       /* unused */
    __TI_PIN(19, M, 3),       /* unused */
    __TI_PIN_DEFAULT(20),     /* unused */
    __TI_PIN_DEFAULT(21),     /* unused */
    __TI_PIN_DEFAULT(22),     /* unused */
    __TI_PIN_DEFAULT(23),     /* unused */
    __TI_PIN_DEFAULT(24),     /* unused */
    __TI_PIN_DEFAULT(25),     /* unused */
    __TI_PIN_DEFAULT(26),     /* unused */
    __TI_PIN_DEFAULT(27),     /* unused */
    __TI_PIN_DEFAULT(28),     /* unused */
    __TI_PIN_DEFAULT(29),     /* unused */
    __TI_PIN_DEFAULT(30),     /* unused */
    __TI_PIN(31, L, 3),       /* unused */
    __TI_PIN(32, L, 2),       /* unused */
    __TI_PIN(33, L, 1),       /* unused */
    __TI_PIN(34, L, 0),       /* unused */
    __TI_PIN_DEFAULT(35),     /* unused */
    __TI_PIN_DEFAULT(36),     /* unused */
    __TI_PIN_DEFAULT(37),     /* unused */
    __TI_PIN_DEFAULT(38),     /* unused */
    __TI_PIN_DEFAULT(39),     /* unused */
    __TI_PIN_DEFAULT(40),     /* unused */

    /*
     * others
     */

    __TI_PIN(41, N, 1),     /* D1 */
    __TI_PIN(42, N, 0),     /* D2 */
    __TI_PIN(43, F, 4),     /* D3 */
    __TI_PIN(44, F, 0),     /* D4 */
    __TI_PIN(45, J, 0),     /* SW1 */
    __TI_PIN(46, J, 1),     /* SW2 */
};

/* ----------------------- Private function prototypes ---------------------*/

static const struct pin_index *get_pin(uint8_t pin);
static void drv_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value);
static int drv_pin_read(rt_device_t dev, rt_base_t pin);
static void drv_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode);
static rt_err_t drv_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                              rt_uint32_t mode, void (*hdr)(void *args), void *args);
static rt_err_t drv_pin_dettach_irq(struct rt_device *device, rt_int32_t pin);
static rt_err_t drv_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled);

const static struct rt_pin_ops drv_pin_ops =
{
    drv_pin_mode,
    drv_pin_write,
    drv_pin_read,
    drv_pin_attach_irq,
    drv_pin_dettach_irq,
    drv_pin_irq_enable,
};

/*-------------------------------- Functions -------------------------------*/

int rt_hw_pin_init(void)
{
    int result;
    result = rt_device_pin_register("pin", &drv_pin_ops, RT_NULL);
    return result;
}
INIT_BOARD_EXPORT(rt_hw_pin_init);

/*-------------------------------- Private  Functions -------------------------------*/

static const struct pin_index *get_pin(uint8_t pin)
{
    const struct pin_index *index;
    if (pin < ITEM_NUM(pins))
    {
        index = &pins[pin];
        if (index->index == -1)
            index = RT_NULL;
    }
    else
    {
        index = RT_NULL;
    }
    return index;
};

static void drv_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    const struct pin_index *index;
    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    GPIOPinWrite(index->gpio_port, index->gpio_pin, value == PIN_HIGH ? index->gpio_pin : 0);
}

static int drv_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value;
    const struct pin_index *index;
    value = PIN_LOW;
    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return value;
    }
    value = GPIOPinRead(index->gpio_port, index->gpio_pin) != 0 ? PIN_HIGH : PIN_LOW;
    return value;
}

static void drv_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    const struct pin_index *index;

    uint32_t dir_mode = GPIO_DIR_MODE_IN;
    uint32_t strength = GPIO_STRENGTH_2MA;
    uint32_t pin_type = GPIO_PIN_TYPE_STD;

    index = get_pin(pin);
    if (index == RT_NULL)
    {
        return;
    }

    /* GPIO Periph clock enable */
    SysCtlPeripheralEnable(index->gpio_periph);
    while(!SysCtlPeripheralReady(index->gpio_periph));

    /* Configure GPIO_InitStructure */
    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        dir_mode = GPIO_DIR_MODE_OUT;
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        pin_type = GPIO_PIN_TYPE_STD_WPU;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        pin_type = GPIO_PIN_TYPE_STD_WPD;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        dir_mode = GPIO_DIR_MODE_OUT;
        pin_type = GPIO_PIN_TYPE_OD;
    }

    GPIOPadConfigSet(index->gpio_port, index->gpio_pin, strength, pin_type);
    GPIODirModeSet(index->gpio_port, index->gpio_pin, dir_mode);
}

static rt_err_t drv_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                              rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    return -RT_ENOSYS;
}

static rt_err_t drv_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    return -RT_ENOSYS;
}

static rt_err_t drv_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled)
{
    return -RT_ENOSYS;
}
