/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __AP_VisualOdom_AP_VisualOdom_h__
#define __AP_VisualOdom_AP_VisualOdom_h__


#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Param/AP_Param.h>
#include <AP_Math/AP_Math.h>
#include <AP_SerialManager/AP_SerialManager.h>

class AP_VisualOdom_Backend;

#define AP_VISUALODOM_TIMEOUT_MS 300

class AP_VisualOdom
{
public:
    friend class AP_VisualOdom_Backend;

    AP_VisualOdom();

    // external position backend types (used by _TYPE parameter)
    enum AP_VisualOdom_Type {
        AP_VisualOdom_Type_None   = 0,
        AP_VisualOdom_Type_MAV    = 1
    };

    // The VisualOdomState structure is filled in by the backend driver
    struct VisualOdomState {
        Vector3f angle_delta;       // attitude delta (in radians) of most recent update
        Vector3f position_delta;    // position delta (in meters) of most recent update
        uint64_t time_delta_usec;   // time delta (in usec) between previous and most recent update
        float confidence;           // confidence expressed as a value from 0 (no confidence) to 100 (very confident)
        uint32_t last_update_ms;    // system time (in milliseconds) of last update from sensor
    };

    // detect and initialise any sensors
    void init();

    // return true if sensor is enabled
    bool enabled() const;

    // return true if sensor is basically healthy (we are receiving data)
    bool healthy() const;

    // state accessors
    const Vector3f &get_angle_delta() const { return _state.angle_delta; }
    const Vector3f &get_position_delta() const { return _state.position_delta; }
    uint64_t get_time_delta_usec() const { return _state.time_delta_usec; }
    float get_confidence() const { return _state.confidence; }
    uint32_t get_last_update_ms() const { return _state.last_update_ms; }

    // return a 3D vector defining the position offset of the camera in meters relative to the body frame origin
    const Vector3f &get_pos_offset(void) const { return _pos_offset; }

    // consume VISUAL_POSITION_DELTA data from MAVLink messages
    void handle_msg(mavlink_message_t *msg);

    static const struct AP_Param::GroupInfo var_info[];

private:

    // parameters
    AP_Int8 _type;
    AP_Vector3f _pos_offset;    // position offset of the camera in the body frame
    AP_Int8 _orientation;       // camera orientation on vehicle frame

    // reference to backends
    AP_VisualOdom_Backend *_driver;

    // state of backend
    VisualOdomState _state;
};

#endif /* __AP_VisualOdom_AP_VisualOdom_h__ */