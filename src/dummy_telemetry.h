
#pragma once

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module simulating a telemetry transmission channel.
    It outputs all received messages to the USB serial connection.
*/
class DummyTelemetry : public Module
{

public:

    // constructor
    DummyTelemetry(uint32_t baud);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It switches on the LED and sends a message to all registered receivers.
    virtual void run();

    // port over which position data is sent out at requested rate
    ReceiverPort<MESSAGE_TELEMETRY> output;
    
private:

    uint32_t    last_transmission;
    
};
