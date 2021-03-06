/*
 * board_config.c
 *
 *  Created on: 2019Äê3ÔÂ29ÈÕ
 *      Author: Administrator
 */

#include "board_config.h"
#include <rtthread.h>

/*----------------------------- Private define -------------------------------*/



/*----------------------------- Private typedef ------------------------------*/



/*------------------------------ Private variables ---------------------------*/


/* ----------------------- Private function prototypes ---------------------*/



/*-------------------------------- Functions -------------------------------*/

const char *px4_spi_bus_name(int bus_num)
{
    switch(bus_num)
    {
    case PX4_SPI_BUS_SENSORS:
        return "spi1";
    default:
        return "unknown";
    }
}

/*-------------------------------- Private  Functions -------------------------------*/
