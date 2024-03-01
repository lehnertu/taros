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
    health_delay_ms = repetition_ms;
    max_isr_duration = 0;
    runlevel_ = MODULE_RUNLEVEL_STOP;
}

void Watchdog::setup()
{
    health_delay_counter = 0;
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
    runlevel_ = MODULE_RUNLEVEL_OPERATIONAL;
};

void Watchdog::interrupt()
{
    health_delay_counter++;
    if (FC_isr_duration>max_isr_duration)
        max_isr_duration = FC_isr_duration;
    if (health_delay_counter > health_delay_ms)
    {
        health_delay_counter=0;
        schedule_task(this, std::bind(&Watchdog::analyze_health, this));
    }
}

void Watchdog::analyze_health()
{
    // report the duration of the interrupt calls
    std::stringstream report1;
    report1 << "IRQ total : ";
    report1 << std::fixed << std::setprecision(2) << 1e6*(float)max_isr_duration/(float)F_CPU_ACTUAL << " us";
    report1 << " -- " << FC_max_isr_time_module << " : ";
    report1 << 1e6*(float)FC_max_isr_time_to_completion/(float)F_CPU_ACTUAL << " us";
    float delay = 1.0e6 * (float)FC_max_isr_spacing / (float)F_CPU_ACTUAL;
    report1 << " -- spacing : " << delay << " us";
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report1.str()) );

    // report potentially delayed systick interrupts
    if (delay>1100.0)
    {
        std::stringstream report2;
        report2 << "delayed systick (spacing ";
        report2 << std::fixed << std::setprecision(1);
        report2 << delay << " us";
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, report2.str()) );
    }
    
    // report potentially delayed task starts
    delay = 1.0e6 * (float)FC_max_task_delay / (float)F_CPU_ACTUAL;
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
    report4 << FC_max_task_runtime_module << " : ";
    report4 << std::fixed << std::setprecision(1) << 1e6*(float)FC_max_task_runtime/(float)F_CPU_ACTUAL << " us";
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report4.str()) );
    
    max_isr_duration = 0;
    FC_max_isr_time_to_completion = 0;
    FC_max_isr_spacing = 0;
    FC_max_task_delay = 0;
    FC_max_task_runtime = 0;
    
}

