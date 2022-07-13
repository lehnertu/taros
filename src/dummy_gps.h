
#pragma once

#include <string>

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
        std::string name,    // the ID of the module
        float rate,          // the GPS update rate
        float tm_rate        // the rate at which telemetry messages are sent
            );
    
    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It switches on the LED and sends a message to all registered receivers.
    virtual void run();

    // destructor
    virtual ~DummyGPS() {};

    // assemble a position message from current data upon request
    Message get_position();

    // port over which position data is sent out at requested rate
    SenderPort output;
    
    // port over which telemetry messages are sent
    SenderPort tm_out;

    // port over which status messages are sent
    SenderPort status_out;

private:

    double      lat; 		// latitude in degree (north positive)
    double      lon;		// longitude in degree (east positive)
    float       alt;		// altitude in m

    float       vx;         // simulated velocity in east direction [m/s]
    float       vy;         // simulated velocity in north direction [m/s]
    float       vz;         // simulated velocity in up direction [m/s]

    uint32_t    startup_time;
    float       gps_rate;
    uint32_t    last_update;
    float       telemetry_rate;
    uint32_t    last_telemetry;
    
    // here are some flags indicating which work is due
    bool        flag_state_change;
    bool        flag_update_pending;
    bool        flag_telemetry_pending;
    
    bool        status_lock;
};
