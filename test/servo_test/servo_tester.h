
#pragma once

#include "global.h"
#include "message.h"
#include "module.h"


/*  
    Make some servos move by giving setpoints to the driver module.
*/
class ServoTester : public Module
{

public:

    // Constructor for the ServoTester object.
    // The mask contains one bit for every servo channel indicating
    // whether it should be moved (1) or left alone (0).
    ServoTester(
        std::string name,    // the ID of the module
        uint8_t mask
    );

    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // TODO: This can be removed as soon as all modules exclusively use the inetrrupt method
    virtual bool have_work()  { return false; };

    // This gets called once per millisecond from the systick interrupt.
    // Every 10 ms a task to transmit new servo setpoints is scheduled.
    virtual void interrupt();
    
    // Task function to compute and send setpoints.
    void update();

    // TODO: This can be removed as soon as all modules exclusively use the inetrrupt method
    virtual void run() {};

    // port over which position data is sent out
    SenderPort servo_out;

private:

    uint32_t    last_update_time;
    uint8_t     servo_mask;
    int         counter;        // running time in 10 ms steps
    // phase running 0 to 199
    const int   zero_phase[NUM_SERVO_CHANNELS] = {0,50,100,150,25,75,125,175};
    short int   values[NUM_SERVO_CHANNELS] = {0};
    
};
