
#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module simulating a telemetry transmission channel.
    It outputs all received messages to the USB serial connection.
*/
class USB_Serial : public Module
{

public:

    // constructor
    USB_Serial(
        std::string name,
        uint32_t baud_rate);
    
    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~USB_Serial() {};

    // port at which text messages are received to be sent over the USB serial connection
    ReceiverPort in;

private:

    // here are some flags indicating which work is due
    bool  flag_msg_pending;

};
