#include "global.h"

#include <stdio.h>
#include "cpp_timer.h"
#include <unistd.h>
#include <time.h>
#include <thread>

volatile uint16_t FC_ms_count;
volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
volatile uint8_t FC_systick_flag;

// These 2 variables are part of the Teensyduino core.
// They are onlc included for the systick interupt service routine.
extern "C" volatile uint32_t systick_cycle_count;
extern "C" volatile uint32_t systick_millis_count;

void SystickTimer::timerEvent()
{
    FC_systick_millis_count++;
    FC_ms_count++;
    if (FC_ms_count >= 1000) FC_ms_count=0;
    FC_systick_flag++;
}

SystickTimer timer;

void setup_core_system()
{
    FC_systick_flag = 0;
    FC_systick_millis_count = 0;
    FC_ms_count = 0;
	timer.startns(1000000);
}

Logger system_log("SYSLOG");


