/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

// for debugging USB output
// # include <string>

#include "global.h"
#include "base.h"
#include "servo_tester.h"

ServoTester::ServoTester(
    std::string name,
    uint8_t mask )
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    servo_mask = mask;
    last_update_time = FC_time_now();
    counter = 0;
}

void ServoTester::interrupt()
{
    float elapsed = FC_elapsed_millis(last_update_time);
    if (elapsed >= 10.0)
    {
        counter++;
        schedule_task(this, std::bind(&ServoTester::update, this));
        last_update_time = FC_time_now();
    };
}

void ServoTester::update()
{
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
        short int val = values[i];
        int phase = (counter-zero_phase[i]) % 200;
        if (phase == 0)
            val = 1000;
        else
        {
            if (phase<=100)
                val -= 20;
            else
                val += 20;
        }
        values[i] = val;
    };
    // compile a message
    Message msg(id, MSG_TYPE_SERVO, sizeof(values), (void*) values);
    servo_out.transmit(msg);
    // debugging printout
    /*
    std::string report("servo : ");
    char rs[80];
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
        sprintf(rs, "%5d ", values[i]);
        report += std::string(rs); 
    };
    system_log->in.receive(Message::TextMessage(id, report));
    */
}

