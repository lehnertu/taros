/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include <cstdlib>
#include <cstdint>
#include <list>

#include "module.h"
#include "message.h"
#include "system.h"

// counting the miliseconds within one second
volatile uint16_t ms_count;

// uint32_t systick_millis_count is used as timestamp
// miliseconds since program start (about 50 days capacity)

// raise a flag for the main loop to trigger scheduler/taskmanager execution
volatile uint8_t systick_flag = 0;

// we use our own ISR for the systick interrupt
// it is copied from EventRecorder.cpp (previously startup.c)
// and added with our own functionality
extern "C" void modified_systick_isr(void)
{
    // --- begin original code
	// systick_cycle_count = ARM_DWT_CYCCNT;
	systick_millis_count++;
	// --- end original code
	ms_count++;
	if (ms_count >= 1000) ms_count=0;
	systick_flag++;
}


extern "C" int main(void)
{

    // system-wide initializations
	pinMode(13, OUTPUT);

    // all modules are registered in a list
    std::list<Module*> module_list;
    
    // now create and wire all modules and add them to the list
    // all extended initializations are not yet done but
    // have to be handled my the modules as tasks
    build_system(&module_list);

    // bend the systick ISR to our own
    _VectorsRam[15] = &modified_systick_isr;

	// infinite system loop
	while (1)
	{
	
	    // systick interrupt has occured - run scheduler
	    if (systick_flag>0)
	    {
	        // a value greater than one indicates that one systick has been skipped
	        // which should be reported as a major instability incident
	        if (systick_flag>1) {};
	        // reset the flag
	        systick_flag=0;
	        
	        // The scheduler runs through the complete list of modules.
	        // All modules which have work to do get a task scheduled
	        // at appropriate position (priority) in the (ordered) task list.
	    };
	    
	    // taskmanager:
	    // All scheduled tasks get executed based on priority (list position).
	    
		if (ms_count>700)
		    digitalWriteFast(13, HIGH);
		else
    		digitalWriteFast(13, LOW);

	}

}

