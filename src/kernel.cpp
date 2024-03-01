#include "kernel.h"
#include "module.h"

// temporary includes needed before cleaning up the kernel_loop()
#include "system.h"
#include <cstdlib>


/***** the core timing/interrupt system *****/

volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
volatile uint32_t FC_max_isr_spacing;
volatile uint32_t FC_isr_duration;
volatile uint32_t FC_max_isr_time_to_completion;
std::string FC_max_isr_time_module;
volatile uint32_t FC_max_task_runtime;
std::string FC_max_task_runtime_module;
volatile uint32_t FC_max_task_delay;

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
    __disable_irq();
    // we keep the original code in place
    // in order not to break functionality of the core
    // --- begin original code
    systick_cycle_count = ARM_DWT_CYCCNT;
    systick_millis_count++;
    // --- end original code
    uint32_t last_count = FC_systick_cycle_count;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    FC_systick_millis_count++;
    // keep track of potentially delayed interrupts
    uint32_t spacing = FC_systick_cycle_count-last_count;
    if (spacing > FC_max_isr_spacing) FC_max_isr_spacing=spacing;
    // call all module interrupts - record timing
    std::list<Module*>::iterator it;
    for (it = module_list.begin(); it != module_list.end(); it++)
    {
        Module* mod = *it;
        // we check timing for every module call
        uint32_t isr_start = ARM_DWT_CYCCNT;
        // call the modules interrupt procedure
        mod->interrupt();
        uint32_t isr_stop = ARM_DWT_CYCCNT;
        // the difference automaticall wraps around
        uint32_t cycles = isr_stop - isr_start;
        // the worst module ist stored for reporting by the watchdog
        // the watchdog periodically resets the max value to 0
        if (cycles>FC_max_isr_time_to_completion)
        {
            FC_max_isr_time_module = mod->id;
            FC_max_isr_time_to_completion = cycles;
        };
    };
    // record the total time the interrupt took
    FC_isr_duration = ARM_DWT_CYCCNT - FC_systick_cycle_count;
    __enable_irq();
}

void setup_core_system()
{
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
        .request_time = ARM_DWT_CYCCNT,
        .funct = f
        };
    task_list.push_back(task);
}

void kernel_loop()
{
	while(true)
	{
	
	    // delayMicroseconds(10);
	    
        // TODO: removing this watchdog code breaks the system -- why ???
        
        // maybe some litte delay is necessary before we can check the tasklist again,
        // otherwise overrunning it ?
        // the delay for 10 us did not improve it
        // or could we be finding something in the tasklist, befoer the task definition is actually complete ?
    
        // watchdog - once per second
        if (FC_systick_millis_count % 1000 == 837)
        {
            float delay = 1.0e6 * (float)FC_max_isr_spacing / (float)F_CPU_ACTUAL;
            // FC_max_isr_spacing = 0;
            if (delay>1100.0)
            {
                // even deleting just this fraction of code (that is never executed ?!)
                // breaks the system !!!
                
                char numStr[20];
                sprintf(numStr,"%7.1f", delay);
                std::string text("delayed systick (spacing "+std::string(numStr)+" us)!");
                system_log->in.receive(
                    Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_CRITICAL, text) );
            }
        }

	    // TASKMANAGER:
	    // All scheduled tasks get executed based on priority (list position).
	    // It should be guaranteed that any scheduled task is executed within 10 ms.
        // No task should run longer than 5ms.
        // Both constraints are not actively inforced, but any violations are reported.
        
        if (task_list.empty())
        {
            // this is a busy wait for 10us determined by the number of CPU cycles elapsed
            delayMicroseconds(10);
        }
        else
        {
            // get the first entry in the task list
            Task task = task_list.front();
            // remove it from the list
            task_list.pop_front();
            // check how much time has elapsed from the request of the task
            uint32_t start_delay = ARM_DWT_CYCCNT-task.request_time;
            // the max is reset when the watchdog checks it
            if (start_delay>FC_max_task_delay) FC_max_task_delay=start_delay;

            // execute the task
            uint32_t start = ARM_DWT_CYCCNT;
            task.funct();
            uint32_t stop = ARM_DWT_CYCCNT;
            // the difference automaticall wraps around
            uint32_t runtime = stop - start;
            // check the runtime of the task
            if (runtime>FC_max_task_runtime)
            {
                // the max is reset when an output is created (either log or display)
                FC_max_task_runtime_module = task.module->id;
                FC_max_task_runtime = runtime;
            };
            // if execution time exceeds 5ms the offending module is reported
            if (runtime>5000)
            {
                /*
                char numStr[20];
                sprintf(numStr,"%d",(int)(stop-start));
                std::string text(task.module->id+" task runtime "+std::string(numStr)+" us");
                system_log->in.receive(
                    Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_CRITICAL, text) );
                */
            };
        };

	}; // infinite system loop
}

