/*
  dummy backend for HIL (and SITL). This doesn't actually need to do
  any work, as setHIL() is in the frontend
 */
#ifndef __AP_Baro_AP_Baro_HIL_h__
#define __AP_Baro_AP_Baro_HIL_h__


#include "AP_Baro_Backend.h"

class AP_Baro_HIL : public AP_Baro_Backend
{
public:
    AP_Baro_HIL(AP_Baro &baro);
    void update(void);

private:
    uint8_t _instance;
};

#endif /* __AP_Baro_AP_Baro_HIL_h__ */