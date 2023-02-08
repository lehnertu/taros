
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
    Any values outside this range are ignored which makes it possible
    to set new values for just a few channels but leave the others.
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
    
    // TODO: This can be removed as soon as all modules exclusively use the inetrrupt method
    virtual bool have_work() { return false; };

    // This gets called once per millisecond from the systick interrupt.
    // When necessary worker tasks are scheduled to switch the LED.
    virtual void interrupt();

    // This is the worker function being executed by the taskmanager.
    // It sets the output for all servo channels.
    void handle_message();

    // destructor
    virtual ~Servo8chDriver() {};

    // port at which arbitrary messages are received
    // only MESSAGE_SERVO type messages are evaluated
    ReceiverPort in;

    // TODO: This can be removed as soon as all modules exclusively use the inetrrupt method
    virtual void run() {};
    
    // Set the output values.
    // This could be used during setup before the module can process messages
    // but also later on circumventing the message system.
    void set_pos(short int value[NUM_SERVO_CHANNELS]);
    
    // Activate a set of output channels.
    // The mask contains one bit for every servo channel indicating
    // whether it should be switched on (1) or off (0).
    void activate(uint8_t mask);

private:

    // NUM_SERVO_CHANNELS is defined in <message.h>
    const int pins[NUM_SERVO_CHANNELS] = {2, 3, 4, 5, 7, 8, 28, 29};
    short int current_pos[NUM_SERVO_CHANNELS];
    bool is_active[NUM_SERVO_CHANNELS];
    
};
