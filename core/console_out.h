
#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module simulating a telemetry transmission channel.
    It outputs all received messages to the system console.
*/
class Console_out : public Module
{

public:

    // constructor
    Console_out(std::string name);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Console_out() {};

    // port at which text messages are received to be sent over the USB serial connection
    ReceiverPort in;

private:

    // here are some flags indicating which work is due
    bool  flag_message_pending;

};
