/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include <cstdlib>
#include <cstdint>
#include <list>

#include "global.h"
#include "module.h"
#include "message.h"
#include "system.h"

/*
    This is a task descriptor
    TODO: this needs extensions: priority, maybe different entry points
*/
struct Task 
{
    // the module which has started this task
    Module* module;
    // the system time at which the task has been started (FC_systick_millis_count)
    uint32_t schedule_time;
};

extern "C" int main(void)
{

    // system-wide initializations
	pinMode(13, OUTPUT);
    
    // all modules are registered in a list
    std::list<Module*> module_list;
    
    // all tasks that have been scheduled for execution
    std::list<Task> task_list;

    // now create and wire all modules and add them to the list
    // all extended initializations are not yet done but
    // have to be handled my the modules as tasks
    FC_build_system(&module_list);

    FC_systick_flag = 0;
    // bend the systick ISR to our own
    _VectorsRam[15] = &FC_systick_isr;

	// infinite system loop
	while (1)
	{
	
	    // systick interrupt has occured or tasklist empty - run scheduler
	    if (FC_systick_flag>0 or task_list.empty())
	    {
	        // a value greater than one indicates that one systick has been skipped
	        // which should be reported as a major instability incident
	        if (FC_systick_flag>1)
	        {
	            // TODO
	        };
	        // reset the flag
	        FC_systick_flag=0;
	        
	        // SCHEDULER:
	        // The scheduler runs through the complete list of modules.
	        // All modules which have work to do get a task scheduled
	        // at appropriate position (priority) in the (ordered) task list.
	        std::list<Module*>::iterator it;
	        for (it = module_list.begin(); it != module_list.end(); it++)
	        {
	            Module* mod = *it;
	            if (mod->have_work())
	            {
	                Task task = {
	                    .module = mod,
	                    .schedule_time = FC_systick_millis_count
	                    };
	                task_list.push_back(task);
	                // Serial.print("scheduling Module at ");
	                // Serial.println((uint64_t)task.module);
	            }
	        };
	        
	    };
	    
	    // TASKMANAGER:
	    // All scheduled tasks get executed based on priority (list position).
        if (!task_list.empty())
        {
            // get the first entry in the task list
            Task task = task_list.front();
            // remove it from the list
            task_list.pop_front();
            // Serial.print("calling Module at ");
            // Serial.println((uint64_t)task.module);
            // execute the task
            task.module->run();
        }

	} // infinite system loop

}

