/*
 * dev_eeprom.c
 *
 *  Created on: 2019Äê5ÔÂ9ÈÕ
 *      Author: Administrator
 */

#include "dev_eeprom.h"

#include "board.h"
#include "driverlib/eeprom.h"

/*----------------------------- Private define -------------------------------*/

#define CACHE_SIZE  512

/*----------------------------- Private typedef ------------------------------*/



/*------------------------------ Private variables ---------------------------*/



/* ----------------------- Private function prototypes ---------------------*/

static void dev_eeprom_test(void);

/*-------------------------------- Functions -------------------------------*/

int dev_eeprom_init(void)
{
    uint32_t ret;
    //
    // Enable the EEPROM module.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    //
    // Wait for the EEPROM module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));
    //
    // Wait for the EEPROM Initialization to complete
    //
    ret = EEPROMInit();
    //
    // Check if the EEPROM Initialization returned an error
    // and inform the application
    //
    if(ret != EEPROM_INIT_OK)
    {
        rt_kprintf("EEPROMInit fail!\n");
        return -1;
    }

    rt_kprintf("eeprom size:%d\n", EEPROMSizeGet());

//    dev_eeprom_test();

    return 0;
}
INIT_DEVICE_EXPORT(dev_eeprom_init);

int dev_eeprom_read(rt_uint32_t *dst, rt_uint32_t src, rt_uint32_t n)
{
    if(src % 4 || n % 4 || src + n > EEPROMSizeGet())
    {
        rt_kprintf("invalid param, addr:0x%x, size:0x%x\n", src, n);
        return 0;
    }

    EEPROMRead(dst, src, n);
    return n;
}

int dev_eeprom_write(rt_uint32_t dst, rt_uint32_t *src, rt_uint32_t n)
{
    if(dst % 4 || n % 4 || dst + n > EEPROMSizeGet())
    {
        rt_kprintf("invalid param, addr:0x%x, size:0x%x\n", dst, n);
        return 0;
    }

    EEPROMProgram(src, dst, n);
    return n;
}

//int dev_eeprom_read(void *dst, rt_uint32_t src, rt_uint32_t n)
//{
//    static rt_uint32_t cache[CACHE_SIZE / 4];
//
//    rt_uint32_t total_size = 0;
//    rt_uint32_t once_size;
//    rt_uint32_t aligned_size;
//
//    rt_uint32_t eeprom_size = EEPROMSizeGet();
//
//    if(src >= eeprom_size)
//    {
//        return 0;
//    }
//
//    if(src + n >= eeprom_size)
//    {
//        n = eeprom_size - src;
//    }
//
//    while(n)
//    {
//        once_size = n > CACHE_SIZE ? CACHE_SIZE : once_size;
//        if(once_size % 4)
//        {
//            aligned_size = (once_size / 4 + 1) * 4;
//        }
//        else
//        {
//            aligned_size = once_size;
//        }
//        EEPROMRead(cache, src, aligned_size);
//        rt_memcpy(dst, cache, once_size);
//
//        total_size += once_size;
//        src += once_size;
//        n -= once_size;
//    }
//
//    return total_size;
//}
//
//int dev_eeprom_write(rt_uint32_t dst, void *src, rt_uint32_t n)
//{
//
//}

/*-------------------------------- Private  Functions -------------------------------*/

__attribute__((unused))
static void dev_eeprom_test(void)
{
#define DATA_SIZE   20
#define DATA_ADDR   0x0
    rt_uint32_t data_write[DATA_SIZE];
    rt_uint32_t data_read[DATA_SIZE];
    int ret;

    for(int i = 0; i < DATA_SIZE; i++)
    {
        data_write[i] = i;
    }
    rt_memset(data_read, 0, sizeof(data_read));

    dev_eeprom_write(DATA_ADDR, data_write, sizeof(data_write));
    dev_eeprom_read(data_read, DATA_ADDR, sizeof(data_read));

    ret = rt_memcmp(data_write, data_read, sizeof(data_read));
    rt_kprintf("eeprom test %s\n", ret == 0 ? "ok" : "fail");
}
