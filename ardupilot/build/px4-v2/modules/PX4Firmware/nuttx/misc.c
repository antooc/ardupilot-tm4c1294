/*
 * misc.c
 *
 *  Created on: 2019Äê3ÔÂ13ÈÕ
 *      Author: Administrator
 */

#include "misc.h"
#include "sys/unistd.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

/*----------------------------- Private define -------------------------------*/


/*----------------------------- Private typedef ------------------------------*/

typedef struct
{
	char name[RT_NAME_MAX];
	rt_uint32_t pin;
}led_t;

/*------------------------------ Private variables ---------------------------*/

static led_t led_array[] =
{
		{
				.name = "led0",
				.pin = 41,			/* PN1 */
		},

		{
				.name = "led1",
				.pin = 42,			/* PN0 */
		},

		{
		         .name = "PC7",
		         .pin = 8,
		}
};


/* ----------------------- Private function prototypes ---------------------*/



/*-------------------------------- Functions -------------------------------*/

pid_t getpid(void)
{
    return (pid_t)rt_thread_self();
}

void px4_systemreset(bool hold_in_bootloader)
{
    //TODO px4_systemresets
}

bool up_interrupt_context(void)
{
    rt_base_t level;

    bool ret = false;

    level = rt_hw_interrupt_disable();
    if (rt_interrupt_get_nest() != 0)
    {
        ret = true;
    }
    rt_hw_interrupt_enable(level);

    return ret;
}

int printf(const char *fmt, ...)
{
    va_list args;
    rt_size_t length;
    static char log_buf[RT_CONSOLEBUF_SIZE];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(log_buf, sizeof(log_buf) - 1, fmt, args);
    va_end(args);

    log_buf[sizeof(log_buf) - 1] = '\0';
    rt_kprintf("%s", log_buf);

    return length;
}

void test_led_init(void)
{
	for(int i = 0; i< sizeof(led_array) / sizeof(led_array[0]); i++)
	{
		led_t *led = led_array + i;
		rt_pin_mode(led->pin, PIN_MODE_OUTPUT);
		test_led_turn_off(i);
	}
}

void test_led_turn_on(int num)
{
	if(num < 0 || num >= sizeof(led_array) / sizeof(led_array[0]))
	{
		return;
	}
	rt_pin_write(led_array[num].pin, PIN_HIGH);
}

void test_led_turn_off(int num)
{
	if(num < 0 || num >= sizeof(led_array) / sizeof(led_array[0]))
	{
		return;
	}
	rt_pin_write(led_array[num].pin, PIN_LOW);
}

void test_led_toggle(int num)
{
	if(num < 0 || num >= sizeof(led_array) / sizeof(led_array[0]))
	{
		return;
	}

	rt_pin_write(led_array[num].pin, !rt_pin_read(led_array[num].pin));
}

/*-------------------------------- Private  Functions -------------------------------*/
