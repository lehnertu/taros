/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include "global.h"
#include "blink.h"

Blink::Blink(
    std::string name,
    float rate )
{
    // copy the name
    id = name;
    blink_rate = rate;
    // get access to the LED
	pinMode(13, OUTPUT);
	// initialize the variables
	state_on = false;
    last_on_time = FC_systick_millis_count;
    flag_on_pending=false;
    flag_off_pending = false;
}

bool Blink::have_work()
{
    float elapsed = FC_systick_millis_count - last_on_time;
    if (state_on and (elapsed >= 50.0)) flag_off_pending=true;
    if (elapsed*blink_rate >= 1000.0) flag_on_pending=true;
    // if we need to switch either on or off we have work to do
    return flag_off_pending | flag_on_pending;
}

void Blink::run()
{
    
    if (flag_on_pending)
    {
        last_on_time = FC_systick_millis_count;
        digitalWriteFast(13, HIGH);
        state_on = true;
        flag_on_pending=false;
    };

    if (flag_off_pending)
    {
        digitalWriteFast(13, LOW);
        state_on = false;
        flag_off_pending = false;
    };
 
    
}

