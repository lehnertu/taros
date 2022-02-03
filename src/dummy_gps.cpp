#include "dummy_gps.h"
#include <Arduino.h>

DummyGPS::DummyGPS(
    char const *name,
    float rate )
{
    // copy the name
    strncpy(id,name,8);
    id[8] = 0;
    gps_rate = rate;
    // we assume at this time was the last transmission
    last_transmission = FC_systick_millis_count;
    // home position
    lat = 51.04943;
    lon = 13.89053;
    alt = 285.0;
    vx = 0.0;
    vy = 0.0;
    vz = 3.0;
    // send a status message that we are ready to run
    status_out->transmit(
        MESSAGE_TEXT { .sender_module = id, .text="running OK." }
    );
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

#define DEGREE_PER_METER 9e-6
// int32_t random(void);
// delivers positive numbers running 0...2147483647
#define MAX_RANDOM 2147483648.0

void DummyGPS::run()
{

    // time in seconds
    float elapsed = 0.001*(FC_systick_millis_count - last_transmission);
    // velocity damping
    vx -= 0.2 * vx * elapsed;
    vy -= 0.2 * vy * elapsed;
    vz -= 0.5 * vz * elapsed;
    // random velocity change
    vx += 0.5 * elapsed * random()/MAX_RANDOM;
    vy += 0.5 * elapsed * random()/MAX_RANDOM;
    vz += 1.0 * elapsed * random()/MAX_RANDOM;
    // position change
    lat += vy * elapsed * DEGREE_PER_METER;
    lon += vx * elapsed * DEGREE_PER_METER;
    alt += vz * elapsed;

    // TODO: send message
    
    last_transmission = FC_systick_millis_count;

    // switch on the LED
    digitalWriteFast(13, HIGH);
}

