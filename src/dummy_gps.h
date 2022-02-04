
#pragma once

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"


/*  
    This is a module simulating a GPS sensor.
    It can send MESSAGE_GPS_POSITION at regular intervals (10 Hz).
*/
class DummyGPS : public Module
{

public:

    // constructor sets the rate
    DummyGPS(
        char const *name,
        float rate);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It switches on the LED and sends a message to all registered receivers.
    virtual void run();

    // port over which position data is sent out at requested rate
    SenderPort<MESSAGE_GPS_POSITION> output;
    
    // port over which status messages are sent
    SenderPort<MESSAGE_TEXT> status_out;

private:

    float       lat; 		// latitude in degree (north positive)
    float       lon;		// longitude in degree (east positive)
    float       alt;		// altitude in m

    float       vx;         // simulated velocity in east direction [m/s]
    float       vy;         // simulated velocity in north direction [m/s]
    float       vz;         // simulated velocity in up direction [m/s]

    float       gps_rate;
    uint32_t    last_transmission;
    
};
