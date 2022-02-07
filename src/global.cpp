#include "global.h"
#include <Arduino.h>

volatile uint16_t FC_ms_count;
volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
volatile uint8_t FC_systick_flag;

// These 2 variables are part of the Teensyduino core.
// They are onlc included for the systick interupt service routine.
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
    FC_ms_count++;
    if (FC_ms_count >= 1000) FC_ms_count=0;
    FC_systick_flag++;
}

void setup_core_system()
{
    FC_systick_flag = 0;
    FC_systick_millis_count = 0;
    FC_ms_count = 0;
    FC_systick_cycle_count = ARM_DWT_CYCCNT;
    // bend the systick ISR to our own
    _VectorsRam[15] = &FC_systick_isr;
}

Logger system_log("SYSLOG");


