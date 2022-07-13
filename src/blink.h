
#pragma once

#include "global.h"
#include "module.h"


/*  
    Make the LED on the Teensy board blink.
*/
class Blink : public Module
{

public:

    // constructor sets the rate
    Blink(
        std::string name,    // the ID of the module
        float rate           // the blink rate
            );

    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It switches the LED on or off.
    virtual void run();

private:

    float       blink_rate;
    bool        state_on;
    uint32_t    last_on_time;
    
    // It is quite a bit overkill to set flags when the LED needs to be toggled
    // and have a worker funktion scheduled to actually do it.
    // This serves as an example for more expensive modules where this approach is necessary.
    bool        flag_on_pending;
    bool        flag_off_pending;

};
