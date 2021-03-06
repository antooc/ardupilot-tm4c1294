#ifndef __AP_VisualOdom_AP_VisualOdom_MAV_h__
#define __AP_VisualOdom_AP_VisualOdom_MAV_h__


#include "AP_VisualOdom_Backend.h"

class AP_VisualOdom_MAV : public AP_VisualOdom_Backend
{

public:
    // constructor
    AP_VisualOdom_MAV(AP_VisualOdom &frontend);

    // consume VISION_POSITION_DELTA MAVLink message
    void handle_msg(mavlink_message_t *msg) override;
};

#endif /* __AP_VisualOdom_AP_VisualOdom_MAV_h__ */