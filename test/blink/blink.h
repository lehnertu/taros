
#pragma once

#include "global.h"
#include "module.h"


/*  
    Make the LED on the Teensy board blink.
    This conflicts with the OLED - it uses its SCL pin
*/
class Blink : public Module
{

public:

    // constructor sets the rate
    Blink(
        std::string name,    // the ID of the module
        float rate           // the blink rate in Hz
            );

    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    virtual bool have_work()  { return false; };

    // This gets called once per millisecond from the systick interrupt.
    // When necessary worker tasks are scheduled to switch the LED.
    virtual void interrupt();
    
    // These are the worker function being executed by the taskmanager.
    // They switch the LED on or off.
    // It is quite a bit overkill to use worker functions for this - immediate
    // switching could well be performed within the timing constraints of the interrupt.
    // This serves as an example for more expensive modules where this approach is necessary.
    void switch_on();
    void switch_off();
    
    virtual void run() {};

    
private:

    float       blink_rate;
    bool        state_on;
    uint32_t    last_on_time;
    
};
