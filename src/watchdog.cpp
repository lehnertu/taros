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
    std::stringstream report;
    // report the duration of the interrupt calls
    report << "IRQ total : ";
    report << std::fixed << std::setprecision(2) << 1e6*(float)max_isr_duration/(float)F_CPU_ACTUAL << " us";
    report << " -- " << FC_max_isr_time_module << " : ";
    report << 1e6*(float)FC_max_isr_time_to_completion/(float)F_CPU_ACTUAL << " us";
    
    // report potentially delayed systick interrupts
    float delay = 1.0e6 * (float)FC_max_isr_spacing / (float)F_CPU_ACTUAL;
    report << " -- spacing : " << delay << " us";
    if (delay>1100.0)
    {
        report = std::stringstream("delayed systick (spacing ");
        report << std::fixed << std::setprecision(1);
        report << delay << " us";
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, report.str()) );
    }
    
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATUSREPORT, report.str()) );

    max_isr_duration = 0;
    FC_max_isr_time_to_completion = 0;
    FC_max_isr_spacing = 0;

}

