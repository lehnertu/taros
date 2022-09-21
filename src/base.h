/*  

    This is the core on which TAROS is based.
    
    It handles a list of modules which comprise the application system.
    
    The systick interrupt routine is called every millisecond
    and it in turn calls the interrupt() routine of all modules listed.
    This interrupt routine should only perform minimal work and
    typically return within a few microseconds.
    Time critical work can be done right here, as long as the time constraints are kept.
    The total time needed for the completion of the interrupt routine is monitored.
    
    Within the core we mangage a task list.
    This is intended for all work that may take longer than a few microseconds.
    Every module via its interrupt routine can insert tasks into that list.
    A task consists of a pointer to a static method of the module
    that takes no parameters and returns no values. It only acts on
    the internal state of the module (but could send messages for instance).
    This task list is sequentially processed by the main program.
    
*/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <list>
#include <functional>

#include <module.h>

// uint32_t systick_millis_count is used as timestamp
// miliseconds since program start (about 50 days capacity)
extern volatile uint32_t FC_systick_millis_count;

// ARM_DWT_CYCCNT is a 32-bit unsigned counter for processor cycles (600 MHz)
// we store its value with every systick to procide sub-millisecond timing
extern volatile uint32_t FC_systick_cycle_count;

// we compute the time it took to complete all tasks for one systick
extern uint32_t FC_time_to_completion;
extern uint32_t FC_max_time_to_completion;

// we use our own ISR for the systick interrupt
// it is copied from EventRecorder.cpp (previously startup.c)
// and added with our own functionality
extern "C" void FC_systick_isr(void);

extern uint32_t FC_systick_flag;

// Here are the main initializations that are needed to access the processor hardware.
// 1) bend the interrupt vector to our own ISR
void setup_core_system();

/*
    This is a task descriptor
    TODO: this needs extensions: priority, maybe different entry points
*/
typedef std::function<void ()> TaskFunct;
struct Task 
{
    // the module which has started this task
    Module* module;
    // the system time at which the task has been started
    uint32_t schedule_time;
    // a pointer to the procedure to be executed
    TaskFunct funct;
};

// all modules are registered in a list
extern std::list<Module*> module_list;

// all tasks that have been scheduled for execution
extern std::list<Task> task_list;

