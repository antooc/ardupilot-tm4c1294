#ifndef __AP_HAL_AP_HAL_h__
#define __AP_HAL_AP_HAL_h__


#include <stdint.h>

#include "AP_HAL_Namespace.h"
#include "AP_HAL_Boards.h"
#include "AP_HAL_Macros.h"
#include "AP_HAL_Main.h"

/* HAL Module Classes (all pure virtual) */
#include "UARTDriver.h"
#include "AnalogIn.h"
#include "Storage.h"
#include "GPIO.h"
#include "RCInput.h"
#include "RCOutput.h"
#include "Scheduler.h"
#include "Semaphores.h"
#include "Util.h"
#include "OpticalFlow.h"

#if HAL_WITH_UAVCAN
#include "CAN.h"
#endif

#include "utility/Print.h"
#include "utility/Stream.h"
#include "utility/BetterStream.h"

/* HAL Class definition */
#include "HAL.h"

#include "system.h"

using namespace std;

#endif /* __AP_HAL_AP_HAL_h__ */
