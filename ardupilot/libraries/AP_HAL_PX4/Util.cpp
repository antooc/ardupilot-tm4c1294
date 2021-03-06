
#include <AP_HAL/AP_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_PX4
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
//#include <apps/nsh.h>
//#include <fcntl.h>
#include "UARTDriver.h"
//#include <uORB/uORB.h>
//#include <uORB/topics/safety.h>
//#include <systemlib/board_serial.h>
//#include <drivers/drv_gpio.h>
#include <AP_Math/AP_Math.h>
#include "dev_safety_switch.h"

extern const AP_HAL::HAL& hal;

#include "Util.h"
using namespace PX4;

extern bool _px4_thread_should_exit;

/*
  constructor
 */
PX4Util::PX4Util(void) : Util()
{
//    _safety_handle = orb_subscribe(ORB_ID(safety));
}


/*
  start an instance of nsh
 */
bool PX4Util::run_debug_shell(AP_HAL::BetterStream *stream)
{
    //TODO run_debug_shell
//    PX4UARTDriver *uart = (PX4UARTDriver *)stream;
//    int fd;
//
//    // trigger exit in the other threads. This stops use of the
//    // various driver handles, and especially the px4io handle,
//    // which otherwise would cause a crash if px4io is stopped in
//    // the shell
//    _px4_thread_should_exit = true;
//
//    // take control of stream fd
//    fd = uart->_get_fd();
//
//    // mark it blocking (nsh expects a blocking fd)
//    unsigned v;
//    v = fcntl(fd, F_GETFL, 0);
//    fcntl(fd, F_SETFL, v & ~O_NONBLOCK);
//
//    // setup the UART on stdin/stdout/stderr
//    close(0);
//    close(1);
//    close(2);
//    dup2(fd, 0);
//    dup2(fd, 1);
//    dup2(fd, 2);
//
//    nsh_consolemain(0, nullptr);
//
//    // this shouldn't happen
//    hal.console->printf("shell exited\n");
    return true;
}

/*
  return state of safety switch
 */
enum PX4Util::safety_state PX4Util::safety_switch_state(void)
{
//#if !HAL_HAVE_SAFETY_SWITCH
//    return AP_HAL::Util::SAFETY_NONE;
//#endif
//
//    if (_safety_handle == -1) {
//        _safety_handle = orb_subscribe(ORB_ID(safety));
//    }
//    if (_safety_handle == -1) {
//        return AP_HAL::Util::SAFETY_NONE;
//    }
//    struct safety_s safety;
//    if (orb_copy(ORB_ID(safety), _safety_handle, &safety) != OK) {
//        return AP_HAL::Util::SAFETY_NONE;
//    }
//    if (!safety.safety_switch_available) {
//        return AP_HAL::Util::SAFETY_NONE;
//    }
//    if (safety.safety_off) {
//        return AP_HAL::Util::SAFETY_ARMED;
//    }
//    return AP_HAL::Util::SAFETY_DISARMED;

    return dev_safety_switch_is_off() ? AP_HAL::Util::SAFETY_ARMED : AP_HAL::Util::SAFETY_DISARMED;
}

void PX4Util::set_system_clock(uint64_t time_utc_usec)
{
    //TODO set_system_clock
//    timespec ts;
//    ts.tv_sec = time_utc_usec/1.0e6f;
//    ts.tv_nsec = (time_utc_usec % 1000000) * 1000;
//    clock_settime(CLOCK_REALTIME, &ts);
}

/*
  display PX4 system identifer - board type and serial number
 */
bool PX4Util::get_system_id(char buf[40])
{
    uint8_t serialid[12];
    memset(serialid, 0, sizeof(serialid));
    //TODO get_board_serial
//    get_board_serial(serialid);
    for(uint32_t i = 0; i < sizeof(serialid); i++)
    {
        serialid[i] = i;
    }
#if defined(CONFIG_ARCH_BOARD_PX4FMU_V1)
    const char *board_type = "PX4v1";
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V3)
    const char *board_type = "PX4v3";
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V2)
    const char *board_type = "PX4v2";
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V4)
    const char *board_type = "PX4v4";
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V4PRO)
    const char *board_type = "PX4v4PRO";
#elif defined(CONFIG_ARCH_BOARD_AEROFC_V1)
    const char *board_type = "AEROFCv1";
#else
    const char *board_type = "PX4v?";
#endif
    // this format is chosen to match the human_readable_serial()
    // function in auth.c
    snprintf(buf, 40, "%s %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X",
             board_type,
             (unsigned)serialid[0], (unsigned)serialid[1], (unsigned)serialid[2], (unsigned)serialid[3],
             (unsigned)serialid[4], (unsigned)serialid[5], (unsigned)serialid[6], (unsigned)serialid[7],
             (unsigned)serialid[8], (unsigned)serialid[9], (unsigned)serialid[10],(unsigned)serialid[11]);
    return true;
}

/**
   how much free memory do we have in bytes.
*/
uint32_t PX4Util::available_memory(void) 
{
    rt_uint32_t total, used, max_used;
    rt_memory_info(&total, &used, &max_used);

    return total - used;
}

/*
  AP_HAL wrapper around PX4 perf counters
 */
PX4Util::perf_counter_t PX4Util::perf_alloc(PX4Util::perf_counter_type t, const char *name)
{
    ::perf_counter_type px4_t;
    switch (t) {
    case PX4Util::PC_COUNT:
        px4_t = ::PC_COUNT;
        break;
    case PX4Util::PC_ELAPSED:
        px4_t = ::PC_ELAPSED;
        break;
    case PX4Util::PC_INTERVAL:
        px4_t = ::PC_INTERVAL;
        break;
    default:
        return nullptr;
    }
    return (perf_counter_t)::perf_alloc(px4_t, name);
}

void PX4Util::perf_begin(perf_counter_t h)
{
    ::perf_begin((::perf_counter_t)h);
}

void PX4Util::perf_end(perf_counter_t h)
{
    ::perf_end((::perf_counter_t)h);
}

void PX4Util::perf_count(perf_counter_t h)
{
    ::perf_count((::perf_counter_t)h);
}

void PX4Util::set_imu_temp(float current)
{
    //TODO set_imu_temp
//    if (!_heater.target || *_heater.target == -1) {
//        return;
//    }
//
//    // average over temperatures to remove noise
//    _heater.count++;
//    _heater.sum += current;
//
//    // update once a second
//    uint32_t now = AP_HAL::millis();
//    if (now - _heater.last_update_ms < 1000) {
//        return;
//    }
//    _heater.last_update_ms = now;
//
//    current = _heater.sum / _heater.count;
//    _heater.sum = 0;
//    _heater.count = 0;
//
//    // experimentally tweaked for Pixhawk2
//    const float kI = 0.3f;
//    const float kP = 200.0f;
//    float target = (float)(*_heater.target);
//
//    // limit to 65 degrees to prevent damage
//    target = constrain_float(target, 0, 65);
//
//    float err = target - current;
//
//    _heater.integrator += kI * err;
//    _heater.integrator = constrain_float(_heater.integrator, 0, 70);
//
//    float output = constrain_float(kP * err + _heater.integrator, 0, 100);
//
//    // hal.console->printf("integrator %.1f out=%.1f temp=%.2f err=%.2f\n", _heater.integrator, output, current, err);
//
//    if (_heater.fd == -1) {
//        _heater.fd = open("/dev/px4io", O_RDWR);
//    }
//    if (_heater.fd != -1) {
//        ioctl(_heater.fd, GPIO_SET_HEATER_DUTY_CYCLE, (unsigned)output);
//    }
   
}

void PX4Util::set_imu_target_temp(int8_t *target)
{
    _heater.target = target;
}

//
//extern "C" {
//    extern void *fat_dma_alloc(size_t);
//    extern void fat_dma_free(void *, size_t);
//}

/*
  allocate DMA-capable memory if possible. Otherwise return normal
  memory.
*/
void *PX4Util::dma_allocate(size_t size)
{
#if 0 //!defined(CONFIG_ARCH_BOARD_PX4FMU_V1)
    return fat_dma_alloc(size);
#else
    return malloc(size);
#endif
}
void PX4Util::dma_free(void *ptr, size_t size)
{
#if 0 //!defined(CONFIG_ARCH_BOARD_PX4FMU_V1)
    fat_dma_free(ptr, size);
#else
    return free(ptr);
#endif
}

#endif // CONFIG_HAL_BOARD == HAL_BOARD_PX4
