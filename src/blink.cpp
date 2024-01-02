/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include "base.h"
#include "blink.h"

Blink::Blink(
    std::string name,
    float rate ) : Module(name)
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    blink_rate = rate;
    // get access to the LED
	pinMode(13, OUTPUT);
	// initialize the variables
	state_on = false;
    last_on_time = FC_time_now();
}

void Blink::interrupt()
{
    float elapsed = FC_elapsed_millis(last_on_time);
    if (state_on and (elapsed >= 50.0))
        schedule_task(this, std::bind(&Blink::switch_off, this));
    if (elapsed*blink_rate >= 1000.0)
        schedule_task(this, std::bind(&Blink::switch_on, this));
}

void Blink::switch_on()
{
    last_on_time = FC_time_now();
    digitalWriteFast(13, HIGH);
    state_on = true;
}

void Blink::switch_off()
{
    digitalWriteFast(13, LOW);
    state_on = false;
}

