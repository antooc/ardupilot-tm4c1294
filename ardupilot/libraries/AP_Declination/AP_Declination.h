#ifndef __AP_Declination_AP_Declination_h__
#define __AP_Declination_AP_Declination_h__


#include <inttypes.h>

/*
 *	Adam M Rivera
 *	With direction from: Andrew Tridgell, Jason Short, Justin Beech
 *
 *	Adapted from: http://www.societyofrobots.com/robotforum/index.php?topic=11855.0
 *	Scott Ferguson
 *	scottfromscott@gmail.com
 *
 */
class AP_Declination
{
public:
    static float            get_declination(float lat, float lon);
private:
    static int16_t          get_lookup_value(uint8_t x, uint8_t y);
};

#endif /* __AP_Declination_AP_Declination_h__ */