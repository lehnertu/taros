
#pragma once

#include <string>

#include "global.h"
#include "module.h"
#include "message.h"
#include "port.h"

#include "bno055.h"

/*  
    This module wraps the IMU (BNO-055) and barometric (BMP-388) sensors.
    It sends MESSAGE_AHRS and MESSAGE_GYRO at a rate of 100 Hz.

    The sensor orientation on the board is:
        x:forward y:left z:up
    We want to use a coordinate system leading to the Tait Bryan angles:
        x:forward y:right z:down
    We have to invert the y and z axes

    Then the rotations about the axis are:
        x: roll (positive to the right, zero is level wings, range +/- 90 deg)
        y: pitch (positive nose up, zero is level flight, range +/- 180 deg)
        z: heading (positive to the right, zero is north, range 0...360 deg)

*/
class MotionSensor : public Module
{

public:

    // constructor sets the rate
    MotionSensor(
        std::string name    // the ID of the module
            );
    
    // initialize the data bus to the sensors and setup the sensors
    virtual void setup();
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // It will have work to do almost every millisecond, as the data transfers are split
    // into several actions that take only microseconds of CPU time.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    virtual void run();

    // destructor
    virtual ~MotionSensor() {};

    // port over which angular data is sent out at requested rate
    SenderPort AHRS_out;
    
    // port over which gyro data is sent
    SenderPort GYR_out;

private:

    double      heading;    // in degree (north=0, east=90)
    double      pitch;      // nose angle of attack in degree (up positive)
    double      roll;		// angle of rotation about the heading direction
                            // usual roll angle in horizontal flight
                            // lean angle (yaw) in hover

    float       gyr_x;      // rate of angular rotation [deg/s] about the forwad direction (roll right positive)
    float       gyr_y;      // rate of angular rotation [deg/s] about the wing axis (pitch up positive)
    float       gyr_z;      // rate of angular rotation [deg/s] about the belly axis (heading positive change or yaw right in hover)

    // variables for the sensor access
    
    BNO055                  *bno055;        // the IMU sensor
    uint8_t                 BNO055_I2C;     // BNO-055 slave address
    bool                    bno055_OK;      // sensor state
    
    BNO055::sAxisAnalog_t   Gyro;           // gyro reading : float x, y, z
    BNO055::sQuaAnalog_t    Quat;           // quaternion : float  w, x, y, z;
    
    // here are some flags indicating which work is done
    
    uint32_t    last_update;    // the time of the last update
    bool        flag_update_pending;
    int         query_state;    // which sensor communication is currently done
                                // this is counting run cycles 1..10 creating the 100 Hz update rate
    
};
