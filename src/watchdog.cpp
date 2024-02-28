#include <iostream>
#include <string>
#include <sstream>

#include "base.h"
#include "watchdog.h"
#include "util.h"

Watchdog::Watchdog(
    std::string name,       // the ID of the module
    uint32_t repetition_ms  // time between 2 health reports in ms
    ) : Module(name)
{
    health_delay_ms = repetition_ms;
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
    if (health_delay_counter > health_delay_ms)
    {
        health_delay_counter=0;
        schedule_task(this, std::bind(&Watchdog::analyze_health, this));
    }
}

void Watchdog::analyze_health()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "health report:") );
}

