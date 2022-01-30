
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
    DummyGPS(float rate);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It switches on the LED and sends a message to all registered receivers.
    virtual void run();
    
private:
    SenderPort<MESSAGE_GPS_POSITION> output;
    float       gps_rate;
    uint32_t    last_transmission;
};
