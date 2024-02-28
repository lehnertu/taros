/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include <cstdio>

#include "kernel.h"
#include "kernel.h"
#include "global.h"
#include "servo.h"

Servo8chDriver::Servo8chDriver(
    std::string name
    ) : Module(name)
{
    // set the port pins as input (not active)
    activate(0);
	
	// a value of 0 is mapped to 1.5 ms pulse length
	// 12-bit resolution at 100 Hz yield a time step of 2.44 µs
	// the center 1.5 ms is mapped to 614.4 steps
	// an input step corresponding to 0.5µs mappes to 0.2048 output steps
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
        pinMode(pins[i], INPUT_PULLDOWN);
        is_active[i] = false;
        // for testing every channel gets a different intial value
        // differing by 200 steps channel to channel
        // TODO: finally we will need sane presets
    	double pwm = 614.4 + (i-4)*200 * 0.2048;
    	current_pos[i] = (short int)round(pwm);
	};
}

void Servo8chDriver::interrupt()
{
    // a message is pending - schedule handler
    if (in.count()>0)
        schedule_task(this, std::bind(&Servo8chDriver::handle_message, this));
}

void Servo8chDriver::handle_message()
{
    // we go through all messages pending
    while (in.count()>0)
    {
        Message msg = in.fetch();
        // handle only servo messages
        if (msg.type() == MSG_TYPE_SERVO)
        {
            // get a pointer to the data struct
            MSG_DATA_SERVO *data = (MSG_DATA_SERVO *) msg.get_data();
            // update the settings
            set_pos(data->pos);
        }
    }
}

void Servo8chDriver::set_pos(short int value[NUM_SERVO_CHANNELS])
{
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
        // update those channels that are within sane limits
        short int setpoint = value[i];
        if ((setpoint>=-1000) and (setpoint<=1000) and is_active[i])
        {
        	double pwm = 614.4 + setpoint * 0.2048;
        	current_pos[i] = (short int)round(pwm);
        	analogWrite(pins[i], current_pos[i]);
        };
    };
}

void Servo8chDriver::activate(uint8_t mask)
{
    // set the port pins as output
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
        uint8_t test_bit = 1 << i;
        if ((mask & test_bit) != 0)
        {
            is_active[i] = true;
    	    pinMode(pins[i], OUTPUT);
            analogWriteResolution(12);
            analogWriteFrequency(pins[i], 100);
            analogWrite(pins[i], current_pos[i]);
	    }
    	else
    	{
    	    is_active[i] = false;
    	    pinMode(pins[i], INPUT_PULLDOWN);
	    };
    };
}

