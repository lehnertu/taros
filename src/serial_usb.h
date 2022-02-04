
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
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // port at which text messages are received to be sent over the USB serial connection
    ReceiverPort<MESSAGE_TEXT> text_in;

    // port at which telemetry messages are received to be sent over the USB serial connection
    ReceiverPort<MESSAGE_TELEMETRY> telemetry_in;

private:

    // how to print the different message types
    std::string serialize_text_message(MESSAGE_TEXT msg);
    std::string serialize_telemetry_message(MESSAGE_TELEMETRY msg);
    
    // here are some flags indicating which work is due
    bool  flag_text_pending;
    bool  flag_telemetry_pending;

};
