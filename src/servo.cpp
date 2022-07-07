/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 */
#include <Arduino.h>

#include <cstdio>

#include "global.h"
#include "servo.h"

Servo8chDriver::Servo8chDriver(
    std::string name
    )
{
    // copy the name
    id = name;
    // set the port pins as output
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
	    pinMode(pins[i], OUTPUT);
	// 12-bit resolution - value 0 to 4095
	analogWriteResolution(12);
	// PWM frequency 100 Hz
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    	analogWriteFrequency(pins[i], 100);
	
	// a value of 0 is mapped to 1.5 ms pulse length
	// 12-bit resolution at 100 Hz yield a time step of 2.44 µs
	// the center 1.5 ms is mapped to 614.4 steps
	// an input step corresponding to 0.5µs mappes to 0.2048 output steps
    for (int i=0; i<NUM_SERVO_CHANNELS; i++)
    {
    	double pwm = 614.4 + (i-4)*200 * 0.2048;
    	analogWrite(pins[i], (int)round(pwm));
	};
    flag_message_pending = false;
}

bool Servo8chDriver::have_work()
{
    // if there is something received we have to handle it
    if (in.count()>0) flag_message_pending = true;
    return flag_message_pending;
}

void Servo8chDriver::run()
{
    while (flag_message_pending)
    {
        Message msg = in.fetch();
        // TODO: set servos
        flag_message_pending = (in.count()>0);
    }
}

