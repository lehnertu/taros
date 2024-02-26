#pragma once

#include "module.h"
#include "port.h"
 
// robot state definitions
#define ROBOT_IDLE                  0

// additional definitions of the commander module state
#define MODULE_RUNLEVEL_COMMANDER_PIC 20

/*
    The commander is the highest control instance in th on-board software.
    The commender receives all commands sent by the ground control 
    and controls the actions taken by the robot.
    
    Here we maintain the top-level state of the robot. At all time exactly one
    motion control algorithm is executed, which one is decided by the commander
    based on the robot state and the motion targets received from ground control
    or from a pre-defined mission plan.
*/
class Commander : public Module
{

public:

    // constructor
    Commander(
        std::string name        // the ID of the module
        );

    virtual void setup();
    
    // when all initializations are done the commander takes over
    // TODO: this should be execute first thing when entering the event loop
    void activate();
    
    virtual void interrupt();
    
    // this is for handling commands that are sent over the uplink from ground control
    virtual void handle_uplink();
    
    // port over which status messages are sent
    SenderPort status_out;

    // port at which command messages are received from the uplink
    ReceiverPort command_in;

private:

    int robot_state;
    
};
