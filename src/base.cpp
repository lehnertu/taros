#include "base.h"
#include "module.h"
#include <Arduino.h>

/***** the core timing/interrupt system *****/

volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
volatile uint32_t FC_max_isr_spacing;

volatile uint32_t FC_max_isr_time_to_completion;
std::string FC_max_isr_time_module;
volatile uint32_t FC_max_task_time_to_completion;
std::string FC_max_task_time_module;

uint32_t FC_systick_flag;

std::list<Module*> module_list;
std::list<Task> task_list;

// These 2 variables are part of the Teensyduino core.
// They are only included for the systick interupt service routine.
extern "C" volatile uint32_t systick_cycle_count;
extern "C" volatile uint32_t systick_millis_count;

uint32_t FC_time_now()
{
    return FC_systick_millis_count;
}

uint32_t FC_elapsed_millis(uint32_t timestamp)
{
    uint32_t now = FC_systick_millis_count;
    if (now>=timestamp)
        return now - timestamp;
    else
        // wrap around
        return (0xFFFFFFFF - timestamp) + now + 1;
}

void FC_systick_isr(void)
{
    // we keep the original code in place
    // in order not to break functionality of the core
    // --- begin original code
    systick_cycle_count = ARM_DWT_CYCCNT;
    systick_millis_count++;
    // --- end original code
    uint32_t last_count = FC_systick_cycle_count;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    // keep track of potentially delayed interrupts
    uint32_t spacing = FC_systick_cycle_count-last_count;
    if (spacing > FC_max_isr_spacing) FC_max_isr_spacing=spacing;
    FC_systick_millis_count++;
    FC_systick_flag++;
    // call all module interrupts - record timing
    std::list<Module*>::iterator it;
    for (it = module_list.begin(); it != module_list.end(); it++)
    {
        Module* mod = *it;
        // we check timing for every module call
        // TODO: report anything longer than 50us (30000 CPU cycles)
        uint32_t isr_start = ARM_DWT_CYCCNT;
        mod->interrupt();
        uint32_t isr_stop = ARM_DWT_CYCCNT;
        // the difference automaticall wraps around
        uint32_t cycles = isr_stop - isr_start;
        if (cycles>FC_max_isr_time_to_completion)
        {
            // the max is reset when an output is created (either log or display)
            FC_max_isr_time_module = mod->id;
            FC_max_isr_time_to_completion = cycles;
        };
    };
}

void setup_core_system()
{
    FC_systick_flag = 0;
    FC_systick_millis_count = 0;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    FC_max_isr_spacing = 0;
    FC_max_isr_time_to_completion = 0;
    // bend the systick ISR to our own
    _VectorsRam[15] = &FC_systick_isr;
}

void schedule_task(Module *mod, TaskFunct f)
{
    Task task = {
        .module = mod,
        .request_time = FC_systick_millis_count,
        .funct = f
        };
    task_list.push_back(task);
}


