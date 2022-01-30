/*
    Global definitions that are needed throuout the system
*/

#pragma once

#include <cstddef>
#include <cstdint>

// counting the miliseconds within one second
extern volatile uint16_t FC_ms_count;

// uint32_t systick_millis_count is used as timestamp
// miliseconds since program start (about 50 days capacity)
extern volatile uint32_t FC_systick_millis_count;

// ARM_DWT_CYCCNT is a 32-bit unsigned counter for processor cycles (600 MHz)
// we store its value with every systick to procide sub-millisecond timing
extern volatile uint32_t FC_systick_cycle_count;

// raise a flag for the main loop to trigger scheduler/taskmanager execution
extern volatile uint8_t FC_systick_flag;

// we use our own ISR for the systick interrupt
// it is copied from EventRecorder.cpp (previously startup.c)
// and added with our own functionality
extern "C" void FC_systick_isr(void);

