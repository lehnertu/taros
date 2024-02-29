#pragma once

#include "module.h"
#include "port.h"
 
/*
    This module analyzes all information available about the running tasks and modules.
    It reports a system health state to the status_out.
    It report timing violations of modules to the status_out.
    
    TODO: It stops stalled modules from running any further.
*/
class Watchdog : public Module
{

public:

    // constructor
    Watchdog(
        std::string name,       // the ID of the module
        uint32_t repetition_ms  // time between 2 health reports in ms
        );

    virtual void setup();
    
    virtual void interrupt();
    
    void analyze_health();
    
    // port over which status messages are sent
    SenderPort status_out;

private:

    uint32_t health_delay_ms;
    uint32_t health_delay_counter;

    uint32_t max_isr_duration;
    
};
