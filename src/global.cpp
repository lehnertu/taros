#include "global.h"
#include <Arduino.h>

volatile uint16_t FC_ms_count;
volatile uint32_t FC_systick_millis_count;
volatile uint32_t FC_systick_cycle_count;
volatile uint8_t FC_systick_flag;

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


