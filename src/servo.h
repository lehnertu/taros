
#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"


/*  
    This is a module for driving 8 servo channels.
    The pulse output is running all the time after the Servo8chDriver has been created.
    It can receive MESSAGE_SERVO to set the output value(s).
    
    The data range is defined as -1000 ... 1000
    which is mapped to an output pulse with 1.0...2.0 ms
*/
class Servo8chDriver : public Module
{

public:

    // constructor
    Servo8chDriver(
        std::string name    // the ID of the module
            );
    
    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a message with new dataset has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It sets the output for all servo channels.
    virtual void run();

    // destructor
    virtual ~Servo8chDriver() {};

    // port at which arbitrary messages are received
    ReceiverPort in;

private:

    // NUM_SERVO_CHANNELS is defined in <message.h>
    const int pins[NUM_SERVO_CHANNELS] = {2, 3, 4, 5, 7, 8, 28, 29};

    // here is the flag indicating that work is due
    bool  flag_message_pending;
    
};
