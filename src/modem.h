#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

/*  
    This is a module encapsulating a transmission channel.
    It sends all received messages to the ground station.
    It receives command messages from the ground station.
*/
class Modem : public Module
{

public:

    // constructor
    Modem(
        std::string name,
        uint32_t baud_rate);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Modem() {};

    // port at which text messages are received to be sent
    ReceiverPort<Message> msg_in;

    // port at which telemetry messages are received to be sent
    ReceiverPort<Message_Telemetry> telemetry_in;

private:

    // here are some flags indicating which work is due
    bool  flag_msg_pending;
    bool  flag_telemetry_pending;

};
