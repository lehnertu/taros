#include "base.h"
#include <Arduino.h>

/***** the core timing/interrupt system *****/

volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
uint32_t FC_time_to_completion;
uint32_t FC_max_time_to_completion;
uint32_t FC_systick_flag;

std::list<Module*> module_list;
std::list<Task> task_list;

// These 2 variables are part of the Teensyduino core.
// They are only included for the systick interupt service routine.
extern "C" volatile uint32_t systick_cycle_count;
extern "C" volatile uint32_t systick_millis_count;

void FC_systick_isr(void)
{
    // we keep the original code in place
    // in order not to break functionality of the core
    // --- begin original code
    systick_cycle_count = ARM_DWT_CYCCNT;
    systick_millis_count++;
    // --- end original code
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    FC_systick_millis_count++;
    FC_systick_flag++;
    // call all module interrupts - record timing
    std::list<Module*>::iterator it;
    for (it = module_list.begin(); it != module_list.end(); it++)
    {
        Module* mod = *it;
        mod->interrupt();
    };
    // TODO : prevent overruns when wrapping around
    FC_time_to_completion = ARM_DWT_CYCCNT - FC_systick_cycle_count;
    // the max is reset when an output is created (either log or display)
    if (FC_time_to_completion > FC_max_time_to_completion)
        FC_max_time_to_completion = FC_time_to_completion;
    // FC_max_time_to_completion is reset when printed to the display
}


void setup_core_system()
{
    FC_systick_flag = 0;
    FC_systick_millis_count = 0;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    // bend the systick ISR to our own
    _VectorsRam[15] = &FC_systick_isr;
}



