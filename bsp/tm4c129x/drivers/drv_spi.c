/*
 * File      : gpio.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author            Notes
 * 2017-11-08     ZYH            the first version
 */
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include "driverlib/ssi.h"

#ifdef RT_USING_SPI

#include "drv_spi.h"

#define SPI_MAX_BITRATE     5000000

struct hw_spi_cs
{
    rt_uint32_t pin;
};

struct hw_spi
{
    struct rt_spi_bus bus;
    struct rt_spi_configuration *cfg;

    rt_uint32_t spi_base;
};

#define SPISTEP(datalen) (((datalen) == 8) ? 1 : 2)

static rt_err_t spitxrx1b(struct hw_spi *hspi, void *rcvb, const void *sndb)
{
    rt_uint32_t padrcv = 0;
    rt_uint32_t padsnd = 0xFF;

    if (!rcvb && !sndb)
    {
        return RT_ERROR;
    }
    if (!sndb)
    {
        sndb = &padsnd;
    }

    while(SSIDataGetNonBlocking(hspi->spi_base, &padrcv));

    if(hspi->cfg->data_width == 8)
    {
        SSIDataPut(hspi->spi_base, *(rt_uint8_t *)sndb);
    }
    else
    {
        SSIDataPut(hspi->spi_base, *(rt_uint16_t *)sndb);
    }

    //
    // Wait until SSI0 is done transferring all the data in the transmit FIFO.
    //
    while(SSIBusy(hspi->spi_base));

    SSIDataGet(hspi->spi_base, &padrcv);
    if(rcvb)
    {
        if(hspi->cfg->data_width == 8)
        {
            *(rt_uint8_t *)rcvb = padrcv;
        }
        else
        {
            *(rt_uint16_t *)rcvb = padrcv;
        }
    }

    return RT_EOK;
}

static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    rt_err_t res;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    struct hw_spi *hspi = (struct hw_spi *)device->bus;
    struct hw_spi_cs *cs = device->parent.user_data;

    if (message->cs_take)
    {
        rt_pin_write(cs->pin, 0);
    }

    const rt_uint8_t *sndb = message->send_buf;
    rt_uint8_t *rcvb = message->recv_buf;
    rt_int32_t length = message->length;
    while (length)
    {
        res = spitxrx1b(hspi, rcvb, sndb);
        if (rcvb)
        {
            rcvb += SPISTEP(hspi->cfg->data_width);
        }
        if (sndb)
        {
            sndb += SPISTEP(hspi->cfg->data_width);
        }
        if (res != RT_EOK)
        {
            break;
        }
        length--;
    }

    /* Wait until Busy flag is reset before disabling SPI */
//    while (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_BSY) == SET)
        ;
    if (message->cs_release)
    {
        rt_pin_write(cs->pin, 1);
    }
    return message->length - length;
}

static void gpio_and_clk_init(rt_uint32_t spi_base)
{
    if(spi_base == SSI0_BASE)
    {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0));
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

        GPIOPinConfigure(GPIO_PA2_SSI0CLK);
        GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);   /* SSI0TX */
        GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);   /* SSI0RX */

        GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_2);
    }
    else if(spi_base == SSI2_BASE)
    {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2));
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));

        GPIOPinConfigure(GPIO_PD3_SSI2CLK);
        GPIOPinConfigure(GPIO_PD1_SSI2XDAT0);   /* SSI2TX */
        GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);   /* SSI2RX */

        GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3);
    }
}

static rt_err_t spi_configure(struct rt_spi_device *device,
                       struct rt_spi_configuration *cfg)
{
    struct hw_spi *hspi = (struct hw_spi *)device->bus;
    hspi->cfg = cfg;

    rt_uint32_t data_width;
    rt_uint32_t protocol;
    rt_uint32_t bit_rate;

    if (cfg->mode & RT_SPI_SLAVE)
    {
        return -RT_ENOSYS;
    }

    if (cfg->mode & RT_SPI_3WIRE)
    {
        return -RT_ENOSYS;
    }

//    if (cfg->data_width >=4 && cfg->data_width <= 16)
    if (cfg->data_width != 8 && cfg->data_width != 16)
    {
        return -RT_ENOSYS;
    }
    data_width = cfg->data_width;

    switch(cfg->mode & (RT_SPI_CPHA | RT_SPI_CPOL))
    {
    case RT_SPI_MODE_0:
        protocol = SSI_FRF_MOTO_MODE_0;
        break;
    case RT_SPI_MODE_1:
        protocol = SSI_FRF_MOTO_MODE_1;
        break;
    case RT_SPI_MODE_2:
        protocol = SSI_FRF_MOTO_MODE_2;
        break;
    case RT_SPI_MODE_3:
        protocol = SSI_FRF_MOTO_MODE_3;
        break;
    default:
        return -RT_ENOSYS;
    }

//    if (cfg->mode & RT_SPI_NO_CS)
//    {
//
//    }
//    else
//    {
//
//    }

    if(SystemCoreClock / cfg->max_hz > (254 * 256))
    {
        return -RT_ENOSYS;
    }
    else
    {
        bit_rate = cfg->max_hz;
    }
    if(bit_rate >= SPI_MAX_BITRATE)
    {
        bit_rate = SPI_MAX_BITRATE;
    }

    if (!(cfg->mode & RT_SPI_MSB))
    {
        return -RT_ENOSYS;
    }

    gpio_and_clk_init(hspi->spi_base);
    SSIConfigSetExpClk(hspi->spi_base, SystemCoreClock, protocol, SSI_MODE_MASTER, bit_rate, data_width);
    SSIEnable(hspi->spi_base);

    return RT_EOK;
}
const struct rt_spi_ops hw_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};


static struct hw_spi spi0 =
{
    .spi_base = SSI0_BASE,
};

static struct hw_spi spi2 =
{
    .spi_base = SSI2_BASE,
};

static rt_err_t drv_spi_register_bus(struct hw_spi *spi, const char *name)
{
    return rt_spi_bus_register(&spi->bus, name, &hw_spi_ops);
}

//cannot be used before completion init
rt_err_t hw_spi_bus_attach_device(rt_uint32_t pin, const char *bus_name, const char *device_name)
{
    struct rt_spi_device *spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    struct hw_spi_cs *cs_pin = (struct hw_spi_cs *)rt_malloc(sizeof(struct hw_spi_cs));
    RT_ASSERT(cs_pin != RT_NULL);
    cs_pin->pin = pin;
    rt_pin_mode(pin, PIN_MODE_OUTPUT);
    rt_pin_write(pin, 1);
    return rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin);
}

int hw_spi_init(void)
{
    rt_err_t result = 0;

    result |= drv_spi_register_bus(&spi0, "spi0");
    result |= drv_spi_register_bus(&spi2, "spi2");

    return result;
}
INIT_BOARD_EXPORT(hw_spi_init);


#endif /*RT_USING_SPI*/
