#include "dummy_gps.h"
#include <Arduino.h>

// raise a flag for the main loop to trigger scheduler/taskmanager execution
extern volatile uint8_t systick_flag;

DummyGPS::DummyGPS(float rate)
{
    gps_rate = rate;
    // we assume at this time was the last transmission
    last_transmission = FC_systick_millis_count;
}

bool DummyGPS::have_work()
{
    // Serial.println("DummyGPS check");
    float elapsed = FC_systick_millis_count - last_transmission;
    
    // TODO: test code - remove
    // after 50 ms switch off the LED
    // this breaks the rule that no action should be performed in have_work()
    if (elapsed>10)
		digitalWriteFast(13, LOW);
        
    return (elapsed*gps_rate >= 1000.0);
}

void DummyGPS::run()
{
    last_transmission = FC_systick_millis_count;
    // TODO: send message
    // switch on the LED
    digitalWriteFast(13, HIGH);
}

