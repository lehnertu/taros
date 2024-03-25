#include "kernel.h"
#include "module.h"

// this is needed to have ARM_DWT_CYCCNT and F_CPU_ACTUAL
#include "../core/core_pins.h"

// These 2 variables are part of the Teensyduino core.
// They are only included for the systick interupt service routine.
extern "C" volatile uint32_t systick_cycle_count;
extern "C" volatile uint32_t systick_millis_count;

// uint32_t FC_systick_millis_count is used as timestamp
// miliseconds since program start (about 50 days capacity)
static volatile uint32_t FC_systick_millis_count;
uint32_t FC_time_now() { return FC_systick_millis_count; };

uint32_t FC_elapsed_millis(uint32_t timestamp)
{
    uint32_t now = FC_systick_millis_count;
    if (now>=timestamp)
        return now - timestamp;
    else
        // wrap around
        return (0xFFFFFFFF - timestamp) + now + 1;
}
// ARM_DWT_CYCCNT is a 32-bit unsigned counter for processor cycles (600 MHz)
// we store its value with every systick to procide sub-millisecond timing
static volatile uint32_t FC_systick_cycle_count;

// we use a flag to indicate if it is allowed to call module interrupts
static volatile bool FC_module_interrupts_active;

// we record the maximum number of CPU cycles between 2 interrupts
// (should be about 600000)
static volatile uint32_t FC_max_isr_spacing;
uint32_t FC_get_max_isr_spacing() { return FC_max_isr_spacing; };
void FC_reset_max_isr_spacing() { FC_max_isr_spacing=0; };

// we record the longest time it takes from scheduling a task
// until the task execution is actually started in CPU cycles
static volatile uint32_t FC_max_task_delay;
uint32_t FC_get_max_task_delay() { return FC_max_task_delay; };
void FC_reset_max_task_delay() { FC_max_task_delay=0; };

// we record the total time the systick interrupt routine needs for completion
// this is updated with every interrupt
static volatile uint32_t FC_max_isr_duration;
uint32_t FC_get_max_isr_duration() { return FC_max_isr_duration; };
void FC_reset_max_isr_duration() { FC_max_isr_duration=0; };

// we record the longest time it took to complete an interrupt request
// we can read the latest value or reset it to zero (used by the watchdog)
// apointer to the slowest module is stored and can be used to report its ID.
static volatile uint32_t FC_max_isr_time_to_completion;
static Module* FC_max_isr_time_module;
uint32_t FC_get_max_isr_time_to_completion() { return FC_max_isr_time_to_completion; };
void FC_reset_max_isr_time_to_completion() { FC_max_isr_time_to_completion=0; };
std::string FC_max_isr_time_module_ID() { return FC_max_isr_time_module->id; };

// we record the longest time it takes to complete a task (in CPU cycles)
// we can read the latest value or reset it to zero (used by the watchdog)
// the identifier of the slowest module is stored
static volatile uint32_t FC_max_task_runtime;
static Module* FC_max_task_runtime_module;
uint32_t FC_get_max_task_runtime() { return FC_max_task_runtime; };
void FC_reset_max_task_runtime() { FC_max_task_runtime=0; };
std::string FC_max_task_runtime_module_ID() { return FC_max_task_runtime_module->id; };

std::list<Module*> module_list;
std::list<Task> task_list;

// we use our own ISR for the systick interrupt
// it is copied from EventResponder.cpp (previously delay.c)
// and added with our own functionality
void FC_systick_isr(void)
{
    // we keep the original code in place
    // in order not to break functionality of the core
    // this was originally in core/delay.c and has moved to core/EventResponder.cpp
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
    // TODO: this should be only enabled by a flag that is initially disabled
    // calling the module interrups should only be enabled when all setup is complete
    if (FC_module_interrupts_active)
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
		        FC_max_isr_time_module = mod;
		        FC_max_isr_time_to_completion = cycles;
		    };
		};
    // record the total time the interrupt took
    uint32_t isr_duration = ARM_DWT_CYCCNT - FC_systick_cycle_count;
    if (isr_duration>FC_max_isr_duration) FC_max_isr_duration=isr_duration;
}

void setup_core_system()
{
    FC_systick_millis_count = 0;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    FC_max_isr_spacing = 0;
    FC_max_isr_time_to_completion = 0;
    FC_module_interrupts_active = false;
    // bend the systick ISR to our own
    _VectorsRam[15] = &FC_systick_isr;
}

void FC_module_interrupts_activate()
{
    FC_module_interrupts_active = true;
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
	
        // TODO: removing this old watchdog code breaks the system -- why ???
        // this line can't be removed
        std::string dummy("old watchdogn code");
        
    	// seems we get a zero pointer dereferenced
    	
		/* TODO: when std::string dummy was removed:            
		CrashReport:
		  A problem occurred at (system time) 0:0:25
		  Code was executing from address 0xD8
		  CFSR: 82
				(DACCVIOL) Data Access Violation
				(MMARVALID) Accessed Address: 0x0 (nullptr)
		  Check code at 0xD8 - very likely a bug!
		  Run "addr2line -e mysketch.ino.elf 0xD8" for filen.
		  		--> /home/ulf/Programming/arduino-1.8.19/hardware/tools/arm/arm-none-eabi/include/c++/5.4.1/bits/char_traits.h:243
		  Temperature inside the chip was 50.28 °C
		  Startup CPU clock speed is 600MHz
		  Reboot was caused by auto reboot after fault or bad interrupt detected
		*/

		// in another case we get a nullptr related to the Task struct in the kernel
        // the variable declared outside the loop does not help - we nedd the access


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
                FC_max_task_runtime_module = task.module;
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

