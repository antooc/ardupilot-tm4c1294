#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_PX4

#include <AP_HAL_Empty/AP_HAL_Empty.h>
#include <AP_HAL_Empty/AP_HAL_Empty_Private.h>

#include "AP_HAL_PX4.h"
#include "AP_HAL_PX4_Namespace.h"
#include "HAL_PX4_Class.h"
#include "Scheduler.h"
#include "UARTDriver.h"
#include "Storage.h"
#include "RCInput.h"
#include "RCOutput.h"
//#include "RCOutput_Tap.h"
#include "AnalogIn.h"
#include "Util.h"
#include "GPIO.h"
#include "I2CDevice.h"
#include "SPIDevice.h"
#include <rtthread.h>

#if HAL_WITH_UAVCAN
#include "CAN.h"
#endif

#include <stdlib.h>
//#include <systemlib/systemlib.h>
#include <nuttx/config.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
//#include <poll.h>
//#include <drivers/drv_hrt.h>
#include <nuttx/drv_hrt.h>
#include <nuttx/misc.h>

#include <rtthread.h>
#include "dev_safety_switch.h"

using namespace PX4;

//static Empty::GPIO gpioDriver;

static PX4Scheduler schedulerInstance;
static PX4Storage storageDriver;
static PX4RCInput rcinDriver;
#if defined(CONFIG_ARCH_BOARD_AEROFC_V1)
static RCOutput_Tap rcoutDriver;
#else
static PX4RCOutput rcoutDriver;
#endif
static PX4AnalogIn analogIn;
static PX4Util utilInstance;
static PX4GPIO gpioDriver;

static PX4::I2CDeviceManager i2c_mgr_instance;
static PX4::SPIDeviceManager spi_mgr_instance;

#if defined(CONFIG_ARCH_BOARD_PX4FMU_V2)
#define UARTA_DEFAULT_DEVICE "uart0"
#define UARTB_DEFAULT_DEVICE "uart4"
#define UARTC_DEFAULT_DEVICE "uart2"
#define UARTD_DEFAULT_DEVICE "uart6"
#define UARTE_DEFAULT_DEVICE "uart7"
#define UARTF_DEFAULT_DEVICE "uart3"
#elif defined(CONFIG_ARCH_BOARD_PX4FMU_V4) || defined(CONFIG_ARCH_BOARD_PX4FMU_V4PRO)
#define UARTA_DEFAULT_DEVICE "/dev/ttyACM0"
#define UARTB_DEFAULT_DEVICE "/dev/ttyS3"
#define UARTC_DEFAULT_DEVICE "/dev/ttyS1"
#define UARTD_DEFAULT_DEVICE "/dev/ttyS2"
#define UARTE_DEFAULT_DEVICE "/dev/ttyS6" // frsky telem
#define UARTF_DEFAULT_DEVICE "/dev/ttyS0" // wifi
#elif defined(CONFIG_ARCH_BOARD_AEROFC_V1)
#define UARTA_DEFAULT_DEVICE "/dev/ttyS1" // Aero
#define UARTB_DEFAULT_DEVICE "/dev/ttyS5" // GPS
#define UARTC_DEFAULT_DEVICE "/dev/ttyS3" // Telem
// ttyS0: ESC
// ttyS2: RC
#define UARTD_DEFAULT_DEVICE "/dev/null"
#define UARTE_DEFAULT_DEVICE "/dev/null"
#define UARTF_DEFAULT_DEVICE "/dev/null"
#else
#define UARTA_DEFAULT_DEVICE "/dev/ttyACM0"
#define UARTB_DEFAULT_DEVICE "/dev/ttyS3"
#define UARTC_DEFAULT_DEVICE "/dev/ttyS2"
#define UARTD_DEFAULT_DEVICE "/dev/null"
#define UARTE_DEFAULT_DEVICE "/dev/null"
#define UARTF_DEFAULT_DEVICE "/dev/null"
#endif

// 3 UART drivers, for GPS plus two mavlink-enabled devices
static PX4UARTDriver uartADriver(UARTA_DEFAULT_DEVICE, "APM_uartA");
static PX4UARTDriver uartBDriver(UARTB_DEFAULT_DEVICE, "APM_uartB");
static PX4UARTDriver uartCDriver(UARTC_DEFAULT_DEVICE, "APM_uartC");
static PX4UARTDriver uartDDriver(UARTD_DEFAULT_DEVICE, "APM_uartD");
static PX4UARTDriver uartEDriver(UARTE_DEFAULT_DEVICE, "APM_uartE");
static PX4UARTDriver uartFDriver(UARTF_DEFAULT_DEVICE, "APM_uartF");

HAL_PX4::HAL_PX4() :
    AP_HAL::HAL(
        &uartADriver,  /* uartA */
        &uartBDriver,  /* uartB */
        &uartCDriver,  /* uartC */
        &uartDDriver,  /* uartD */
        &uartEDriver,  /* uartE */
        &uartFDriver,  /* uartF */
        &i2c_mgr_instance,
        &spi_mgr_instance,
        &analogIn, /* analogin */
        &storageDriver, /* storage */
        &uartADriver, /* console */
        &gpioDriver, /* gpio */
        &rcinDriver,  /* rcinput */
        &rcoutDriver, /* rcoutput */
        &schedulerInstance, /* scheduler */
        &utilInstance, /* util */
        nullptr,    /* no onboard optical flow */
        nullptr)   /* CAN */
{}

bool _px4_thread_should_exit = false;        /**< Daemon exit flag */
static bool thread_running = false;        /**< Daemon status flag */
static rt_thread_t daemon_task;//static int daemon_task;                /**< Handle of daemon task / thread */
bool px4_ran_overtime;

extern const AP_HAL::HAL& hal;

/*
  set the priority of the main APM task
 */
void hal_px4_set_priority(uint8_t priority)
{
//    struct sched_param param;
//    param.sched_priority = priority;
//    sched_setscheduler(daemon_task, SCHED_FIFO, &param);

	rt_thread_control(daemon_task, RT_THREAD_CTRL_CHANGE_PRIORITY, &priority);
}

/*
  this is called when loop() takes more than 1 second to run. If that
  happens then something is blocking for a long time in the main
  sketch - probably waiting on a low priority driver. Set the priority
  of the APM task low to let the driver run.
 */
static void loop_overtime(void *)
{
    hal_px4_set_priority(APM_OVERTIME_PRIORITY);
    px4_ran_overtime = true;
}

static AP_HAL::HAL::Callbacks* g_callbacks;

//static int main_loop(int argc, char **argv)
static void main_loop(void *args)
{
    hal.uartA->begin(921600);
    hal.uartB->begin(38400);
    hal.uartC->begin(57600);
    hal.uartD->begin(57600);
    hal.uartE->begin(57600);
    hal.uartF->begin(115200);
    hal.scheduler->init();

    test_led_init();

    // init the I2C wrapper class
    //TODO PX4_I2C::init_lock
//    PX4_I2C::init_lock();
    
    /*
      run setup() at low priority to ensure CLI doesn't hang the
      system, and to allow initial sensor read loops to run
     */
    hal_px4_set_priority(APM_STARTUP_PRIORITY);

    schedulerInstance.hal_initialized();
    dev_safety_switch_set_ready();

    g_callbacks->setup();
    hal.scheduler->system_initialized();

//    perf_counter_t perf_loop = perf_alloc(PC_ELAPSED, "APM_loop");
//    perf_counter_t perf_overrun = perf_alloc(PC_COUNT, "APM_overrun");
    struct hrt_call loop_overtime_call = {0};

    thread_running = true;

    /*
      switch to high priority for main loop
     */
    hal_px4_set_priority(APM_MAIN_PRIORITY);

    while (!_px4_thread_should_exit) {
//        perf_begin(perf_loop);
        
        /*
          this ensures a tight loop waiting on a lower priority driver
          will eventually give up some time for the driver to run. It
          will only ever be called if a loop() call runs for more than
          0.1 second
         */
        hrt_call_after(&loop_overtime_call, 100000, (hrt_callout)loop_overtime, nullptr);

        g_callbacks->loop();

        if (px4_ran_overtime) {
            /*
              we ran over 1s in loop(), and our priority was lowered
              to let a driver run. Set it back to high priority now.
             */
            hal_px4_set_priority(APM_MAIN_PRIORITY);
//            perf_count(perf_overrun);
            px4_ran_overtime = false;
        }

//        perf_end(perf_loop);

        /*
          give up 250 microseconds of time, to ensure drivers get a
          chance to run. This relies on the accurate semaphore wait
          using hrt in semaphore.cpp
         */
        hal.scheduler->delay_microseconds(250);
    }
    thread_running = false;
//    return 0;
}

static void usage(void)
{
    rt_kprintf("Usage: %s [options] {start,stop,status}\n", SKETCHNAME);
    rt_kprintf("Options:\n");
    rt_kprintf("\t-d  DEVICE         set terminal device (default %s)\n", UARTA_DEFAULT_DEVICE);
    rt_kprintf("\t-d2 DEVICE         set second terminal device (default %s)\n", UARTC_DEFAULT_DEVICE);
    rt_kprintf("\t-d3 DEVICE         set 3rd terminal device (default %s)\n", UARTD_DEFAULT_DEVICE);
    rt_kprintf("\t-d4 DEVICE         set 2nd GPS device (default %s)\n", UARTE_DEFAULT_DEVICE);
    rt_kprintf("\n");
}

typedef void (*px4_main_t)(void *);

rt_thread_t px4_task_spawn_cmd(const char *name,
				       int priority,
				       int scheduler,
				       int stack_size,
				       px4_main_t entry,
				       char *const argv[])
{
	rt_thread_t tid = rt_thread_create(name, main_loop, RT_NULL, stack_size, priority, 5);
	if(!tid)
	{
		return tid;
	}

	rt_thread_startup(tid);
	return tid;
}

void HAL_PX4::run(int argc, char * const argv[], Callbacks* callbacks) const
{
    int i;
    const char *deviceA = UARTA_DEFAULT_DEVICE;
    const char *deviceC = UARTC_DEFAULT_DEVICE;
    const char *deviceD = UARTD_DEFAULT_DEVICE;
    const char *deviceE = UARTE_DEFAULT_DEVICE;

    if (argc < 1) {
        rt_kprintf("%s: missing command (try '%s start')",
               SKETCHNAME, SKETCHNAME);
        usage();
//        exit(1);
        return;
    }

    RT_ASSERT(callbacks);
    g_callbacks = callbacks;

    for (i=0; i<argc; i++) {
        if (strcmp(argv[i], "start") == 0) {
            if (thread_running) {
                rt_kprintf("%s already running\n", SKETCHNAME);
                /* this is not an error */
//                exit(0);
                return;
            }

            uartADriver.set_device_path(deviceA);
            uartCDriver.set_device_path(deviceC);
            uartDDriver.set_device_path(deviceD);
            uartEDriver.set_device_path(deviceE);
            rt_kprintf("Starting %s uartA=%s uartC=%s uartD=%s uartE=%s\n",
                   SKETCHNAME, deviceA, deviceC, deviceD, deviceE);

            _px4_thread_should_exit = false;
            daemon_task = px4_task_spawn_cmd(SKETCHNAME,
                                             APM_MAIN_PRIORITY,
                                             SCHED_FIFO,
                                             APM_MAIN_THREAD_STACK_SIZE,
                                             main_loop,
                                             nullptr);
//            exit(0);
            return;
        }

        if (strcmp(argv[i], "stop") == 0) {
            _px4_thread_should_exit = true;
//            exit(0);
            return;
        }
 
        if (strcmp(argv[i], "status") == 0) {
            if (_px4_thread_should_exit && thread_running) {
                rt_kprintf("\t%s is exiting\n", SKETCHNAME);
            } else if (thread_running) {
                rt_kprintf("\t%s is running\n", SKETCHNAME);
            } else {
                rt_kprintf("\t%s is not started\n", SKETCHNAME);
            }
//            exit(0);
            return;
        }

        if (strcmp(argv[i], "-d") == 0) {
            // set terminal device
            if (argc > i + 1) {
                deviceA = strdup(argv[i+1]);
            } else {
                rt_kprintf("missing parameter to -d DEVICE\n");
                usage();
//                exit(1);
                return;
            }
        }

        if (strcmp(argv[i], "-d2") == 0) {
            // set uartC terminal device
            if (argc > i + 1) {
                deviceC = strdup(argv[i+1]);
            } else {
                rt_kprintf("missing parameter to -d2 DEVICE\n");
                usage();
//                exit(1);
                return;
            }
        }

        if (strcmp(argv[i], "-d3") == 0) {
            // set uartD terminal device
            if (argc > i + 1) {
                deviceD = strdup(argv[i+1]);
            } else {
                rt_kprintf("missing parameter to -d3 DEVICE\n");
                usage();
//                exit(1);
                return;
            }
        }

        if (strcmp(argv[i], "-d4") == 0) {
            // set uartE 2nd GPS device
            if (argc > i + 1) {
                deviceE = strdup(argv[i+1]);
            } else {
                rt_kprintf("missing parameter to -d4 DEVICE\n");
                usage();
//                exit(1);
                return;
            }
        }
    }
 
    usage();
//    exit(1);
}

const AP_HAL::HAL& AP_HAL::get_HAL() {
    static const HAL_PX4 hal_px4;
    return hal_px4;
}

#endif // CONFIG_HAL_BOARD == HAL_BOARD_PX4

