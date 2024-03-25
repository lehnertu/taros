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
    
    // this will be called with the above defined repetition rate
    // the runtime spent in interrupt routines and module tasks is analyzed
    // the general interrupt timing and the longest module runtime are reported
    // timing violations are reported separately
    void analyze_health();
	
    // this will be called with the above defined repetition rate
    // the stack and heap memory used by the application are reported
	void analyze_memory();

private:

	// the time in ms between two reports
    uint32_t rate_ms;
    uint32_t health_delay_counter;
	uint32_t memory_delay_counter;
    
};
