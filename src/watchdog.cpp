#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

// this is needed to have F_CPU_ACTUAL
#include "../core/wiring.h"

#include "kernel.h"
#include "watchdog.h"
#include "util.h"

Watchdog::Watchdog(
    std::string name,       // the ID of the module
    uint32_t repetition_ms  // time between 2 health reports in ms
    ) : Module(name)
{
    rate_ms = repetition_ms;
    runlevel_ = MODULE_RUNLEVEL_STOP;
}

void Watchdog::setup()
{
    health_delay_counter = 0;
    memory_delay_counter = rate_ms / 2;
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
    runlevel_ = MODULE_RUNLEVEL_OPERATIONAL;
};

void Watchdog::interrupt()
{
    health_delay_counter++;
    memory_delay_counter++;
    if (health_delay_counter > rate_ms)
    {
        health_delay_counter=0;
        schedule_task(this, std::bind(&Watchdog::analyze_health, this));
    }
    if (memory_delay_counter > rate_ms)
    {
        memory_delay_counter=0;
        schedule_task(this, std::bind(&Watchdog::analyze_memory, this));
    }
}

void Watchdog::analyze_health()
{
    // report the duration of the interrupt calls
    std::stringstream report1;
    report1 << "IRQ total : ";
    report1 << std::fixed << std::setprecision(2) << 1e6*(float)FC_get_max_isr_duration()/(float)F_CPU_ACTUAL << " us";
    report1 << " -- " << FC_max_isr_time_module_ID() << " : ";
    report1 << 1e6*(float)FC_get_max_isr_time_to_completion()/(float)F_CPU_ACTUAL << " us";
    float delay = 1.0e6 * (float)FC_get_max_isr_spacing() / (float)F_CPU_ACTUAL;
    report1 << " -- spacing : " << delay << " us";
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report1.str()) );

    // report potentially delayed systick interrupts
    if (delay>1100.0)
    {
        std::stringstream report2;
        report2 << "delayed systick (spacing ";
        report2 << std::fixed << std::setprecision(1);
        report2 << delay << " us)";
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, report2.str()) );
    }
    
    // report potentially delayed task starts
    delay = 1.0e6 * (float)FC_get_max_task_delay() / (float)F_CPU_ACTUAL;
    if (delay>1100.0)
    {
        std::stringstream report3;
        report3 << "delayed task start ";
        report3 << std::fixed << std::setprecision(1);
        report3 << delay << " us";
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, report3.str()) );
    }
    
    // report longest module runtime
    std::stringstream report4;
    report4 << "Module runtime -- ";
    report4 << FC_max_task_runtime_module_ID() << " : ";
    report4 << std::fixed << std::setprecision(1) << 1e6*(float)FC_get_max_task_runtime()/(float)F_CPU_ACTUAL << " us";
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report4.str()) );
    
    FC_reset_max_isr_time_to_completion();
    FC_reset_max_isr_spacing();
    FC_reset_max_isr_duration();
    FC_reset_max_task_delay();
    FC_reset_max_task_runtime();
    
}

// the heap memory is used from the bottom memory address upwards
// defined in the core library core/imxrt1062_xxx.ld
extern unsigned long _heap_start;
extern unsigned long _heap_end;
// this points to the first free memory position on the heap
// defined in core/startup.c and initialized with (char *)&_heap_start;
extern char *__brkval;
// this is the top of the stack (filled downwards) as defined at system start
// defined in the core library core/imxrt1062_xxx.ld
extern unsigned long _estack;
// determine the used stack memory
uint32_t stack_used()
{
	// to determine the current stack position we allocate a char on the stack
	char tos;
	return _estack - (unsigned long)&tos;
};

void Watchdog::analyze_memory()
{
    // report heap usage
    // TODO: HEAP from 0x41389527 to 0 used up to 0x20007180 -- that is unlikely, probably the start is wrong
    // memory block should be from 0x2000000 to 0x2007ffff (512kB)
    std::stringstream report;
    report << "HEAP from " << (void *)_heap_start << " to " << (void *)_heap_end;
    report << " used up to " << &__brkval;
    // TODO: something here breaks the system -> stall/reboot
    // report << " (" << __brkval-_heap_start << " bytes used)";
    // report << " -- stack usage " << stack_used() << " bytes";
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report.str()) );
}
