/*
 * File      : drv_uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009-2013 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-05-18     Bernard      The first version for LPC40xx
 * 2014-07-18     ArdaFu       Port to TM4C129X
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "board.h"

#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"

#define DEFAULT_CONFIG  \
{\
    .baud_rate = BAUD_RATE_115200,\
    .bit_order = BIT_ORDER_LSB,\
    .data_bits = DATA_BITS_8,\
    .parity    = PARITY_NONE,\
    .stop_bits = STOP_BITS_1,\
    .invert    = NRZ_NORMAL,\
    .bufsz     = RT_SERIAL_RB_BUFSZ,\
}

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

typedef struct
{
    struct rt_serial_device serial;

    char name[RT_NAME_MAX];

    rt_uint32_t gpio_periph;
    rt_uint32_t gpio_port;
    rt_uint32_t gpio_tx_pin;
    rt_uint32_t gpio_tx_cfg;
    rt_uint32_t gpio_rx_pin;
    rt_uint32_t gpio_rx_cfg;

    rt_uint32_t uart_periph;
    rt_uint32_t uart_base;
    void (*uart_irq_handler)(void);
    rt_uint32_t uart_irq_num;

} uart_device_t;

#define UART_DEV_BEGIN(uart_num, port, rx_pin, tx_pin)  \
uart_device_t uart##uart_num;\
void UART##uart_num##_IRQHandler(void)\
{\
    uart_irqhandler(&uart##uart_num);\
}\
uart_device_t uart##uart_num =\
{\
     .serial.ops = &hw_uart_ops,\
     .serial.config = DEFAULT_CONFIG,\
\
     .name = "uart"#uart_num,\
\
     .gpio_periph = SYSCTL_PERIPH_GPIO##port,\
     .gpio_port = GPIO_PORT##port##_BASE,\
     .gpio_rx_pin = GPIO_PIN_##rx_pin,\
     .gpio_rx_cfg = GPIO_P##port##rx_pin##_U##uart_num##RX,\
     .gpio_tx_pin = GPIO_PIN_##tx_pin,\
     .gpio_tx_cfg = GPIO_P##port##tx_pin##_U##uart_num##TX,\
\
     .uart_periph = SYSCTL_PERIPH_UART##uart_num,\
     .uart_base = UART##uart_num##_BASE,\
     .uart_irq_handler = UART##uart_num##_IRQHandler,\
     .uart_irq_num = INT_UART##uart_num,

#define UART_DEV_END    };

/*
 * predeclare symbols
 */
static const struct rt_uart_ops hw_uart_ops;
static const struct rt_uart_ops dummy_uart_ops;
static void uart_irqhandler(uart_device_t *uart_dev);

UART_DEV_BEGIN(0, A, 0, 1)
    .serial.config.baud_rate = 921600,\
UART_DEV_END

UART_DEV_BEGIN(2, D, 4, 5)
UART_DEV_END

UART_DEV_BEGIN(3, A, 4, 5)
UART_DEV_END

UART_DEV_BEGIN(4, K, 0, 1)
UART_DEV_END

UART_DEV_BEGIN(6, P, 0, 1)
UART_DEV_END

UART_DEV_BEGIN(7, C, 4, 5)
    .serial.config.baud_rate = 921600,\
UART_DEV_END

uart_device_t* uart_array[] =
{
#ifdef RT_USING_UART0
     &uart0,
#endif

#ifdef RT_USING_UART2
     &uart2,
#endif

#ifdef RT_USING_UART3
     &uart3,
#endif

#ifdef RT_USING_UART4
     &uart4,
#endif

#ifdef RT_USING_UART6
     &uart6,
#endif

#ifdef RT_USING_UART7
     &uart7,
#endif

     RT_NULL,   /* avoid empty array */
};


static rt_err_t hw_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{	
	uint32_t config = 0;
    RT_ASSERT(serial != RT_NULL);

    uart_device_t *uart_dev = (uart_device_t *)serial;
    rt_uint32_t hw_base = uart_dev->uart_base;

	MAP_UARTDisable(hw_base);
	// build UART Configuration parameter structure
    switch(cfg->data_bits)
	{
	case DATA_BITS_9:
		// enable 9bit address mode and set DATA_BIT_8
		MAP_UART9BitEnable(hw_base);
	case DATA_BITS_8:
		config |= UART_CONFIG_WLEN_8;
		break;
	case DATA_BITS_7:
		config |= UART_CONFIG_WLEN_7;
		break;
	case DATA_BITS_6:
		config |= UART_CONFIG_WLEN_6;
		break;
	case DATA_BITS_5:
		config |= UART_CONFIG_WLEN_5;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	switch(cfg->parity)
	{
	case PARITY_ODD:
		config |= UART_CONFIG_PAR_ODD;
		break;
	case PARITY_EVEN:
		config |= UART_CONFIG_PAR_EVEN;
		break;
	case PARITY_NONE:
		config |= UART_CONFIG_PAR_NONE;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	switch(cfg->stop_bits)
	{
	case STOP_BITS_1:
		config |= UART_CONFIG_STOP_ONE;
		break;
	case STOP_BITS_2:
		config |= UART_CONFIG_STOP_TWO;
		break;
	default:
		RT_ASSERT(0);
		break;
	}
	
	// Initialize UART0 peripheral with given to corresponding parameter
    MAP_UARTConfigSetExpClk(hw_base, SystemCoreClock, cfg->baud_rate, config);
	MAP_UARTFIFOEnable(hw_base);

	// Enable the UART.
	MAP_UARTEnable(hw_base);
    return RT_EOK;
}

static rt_err_t hw_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    RT_ASSERT(serial != RT_NULL);

    uart_device_t *uart_dev = (uart_device_t *)serial;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        MAP_UARTIntDisable(uart_dev->uart_base, UART_INT_RX | UART_INT_RT);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        MAP_UARTIntEnable(uart_dev->uart_base, UART_INT_RX | UART_INT_RT);
        break;
    }

    return RT_EOK;
}

static int hw_putc(struct rt_serial_device *serial, char c)
{
    RT_ASSERT(serial != RT_NULL);

    uart_device_t *uart_dev = (uart_device_t *)serial;

    MAP_UARTCharPut(uart_dev->uart_base, *((uint8_t *)&c));
    return 1;
}

static int hw_getc(struct rt_serial_device *serial)
{
    RT_ASSERT(serial != RT_NULL);

    uart_device_t *uart_dev = (uart_device_t *)serial;
	
	return MAP_UARTCharGetNonBlocking(uart_dev->uart_base);
}

static const struct rt_uart_ops hw_uart_ops =
{
    hw_configure,
    hw_control,
    hw_putc,
    hw_getc,
};

static void uart_irqhandler(uart_device_t *uart_dev)
{
    uint32_t intsrc;

    /* enter interrupt */
    rt_interrupt_enter();

    /* Determine the interrupt source */
    intsrc = MAP_UARTIntStatus(uart_dev->uart_base, true);

    // Receive Data Available or Character time-out
    if (intsrc & (UART_INT_RX | UART_INT_RT))
    {
        MAP_UARTIntClear(uart_dev->uart_base, intsrc);
        rt_hw_serial_isr(&uart_dev->serial, RT_SERIAL_EVENT_RX_IND);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}

static rt_err_t dummy_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    return RT_EOK;
}

static rt_err_t dummy_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    return RT_EOK;
}

static int dummy_putc(struct rt_serial_device *serial, char c)
{
    return 0;
}
static int dummy_getc(struct rt_serial_device *serial)
{
    return -1;
}

static const struct rt_uart_ops dummy_uart_ops =
{
    dummy_configure,
    dummy_control,
    dummy_putc,
    dummy_getc,
};

struct rt_serial_device serial_dummy;

static rt_err_t hw_uart_unit_init(uart_device_t *uart_dev)
{
    MAP_SysCtlPeripheralEnable(uart_dev->gpio_periph);
    MAP_GPIOPinConfigure(uart_dev->gpio_rx_cfg);
    MAP_GPIOPinConfigure(uart_dev->gpio_tx_cfg);

    MAP_GPIOPinTypeUART(uart_dev->gpio_port, uart_dev->gpio_rx_pin | uart_dev->gpio_tx_pin);
    MAP_SysCtlPeripheralEnable(uart_dev->uart_periph);

    /* preemption = 1, sub-priority = 1 */
    //IntPrioritySet(INT_UART0, ((0x01 << 5) | 0x01));

    /* Enable Interrupt for UART channel */
    UARTIntRegister(uart_dev->uart_base, uart_dev->uart_irq_handler);
    MAP_IntEnable(uart_dev->uart_irq_num);
    MAP_UARTEnable(uart_dev->uart_base);

    /* register UART0 device */
    rt_hw_serial_register(&uart_dev->serial, uart_dev->name,
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart_dev);

    return RT_EOK;
}

static rt_err_t dummy_uart_init(struct rt_serial_device *serial_dummy)
{
    struct serial_configure config;

    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;
    config.bufsz     = RT_SERIAL_RB_BUFSZ;

    serial_dummy->ops = &dummy_uart_ops;
    serial_dummy->config = config;
    rt_hw_serial_register(serial_dummy, "uart_dummy",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          RT_NULL);

    return RT_EOK;
}

int rt_hw_uart_init(void)
{
    for(int i = 0; i < ARRAY_SIZE(uart_array); i++)
    {
        if(uart_array[i])
        {
            hw_uart_unit_init(uart_array[i]);
        }
    }

    dummy_uart_init(&serial_dummy);

	return 0;
}
INIT_BOARD_EXPORT(rt_hw_uart_init);
