
#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module simulating a telemetry transmission channel.
    It outputs all received messages to the USB serial connection.
    
    TODO:
    There seems to be a problem if the USB communication fails.
    If the receiver modem is connected to the same computer
    and no USB receiver is running, the
    flight controller hangs after transmitting the first message.
    If there is no USB connection, sometimes, the first boot fails.
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
    
    virtual void interrupt();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run() {};

    // print an incoming message to the USB serial connection
    virtual void handle_MSG();

    // destructor
    virtual ~USB_Serial() {};

    // port at which text messages are received to be sent over the USB serial connection
    ReceiverPort in;

private:

};
