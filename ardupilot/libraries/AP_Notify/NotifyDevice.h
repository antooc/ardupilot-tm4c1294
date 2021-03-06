#ifndef __AP_Notify_NotifyDevice_h__
#define __AP_Notify_NotifyDevice_h__


#include <AP_Common/AP_Common.h>
#include <GCS_MAVLink/GCS_MAVLink.h>

class AP_Notify;

class NotifyDevice {
public:
    virtual ~NotifyDevice() {}
    // init - initialised the device
    virtual bool init(void) = 0;
    // update - updates device according to timed_updated.  Should be
    // called at 50Hz
    virtual void update() = 0;

    // handle a LED_CONTROL message, by default device ignore message
    virtual void handle_led_control(mavlink_message_t *msg) {}

    // handle a PLAY_TUNE message, by default device ignore message
    virtual void handle_play_tune(mavlink_message_t *msg) {}
    
    // this pointer is used to read the parameters relative to devices
    const AP_Notify *pNotify;
};

#endif /* __AP_Notify_NotifyDevice_h__ */