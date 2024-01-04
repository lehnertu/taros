
#pragma once

#include <string>

#include "global.h"
#include "types.h"
#include "module.h"
#include "message.h"
#include "port.h"
#include "stream.h"

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

    // check if the internal calibration has completed
    // and switch to run state if this has been accomplished
    void check_calibration();
    
    // This hooks into the 1ms systick interrupt service routine.
    // It schedules a check_calibration() task until the module has reached MODULE_RUNLEVEL_OPERATIONAL state.
    virtual void interrupt();
    
    // The module will have work to do almost every millisecond, as the data transfers are split
    // into several actions that take only microseconds of CPU time. We want to transfer data
    // as fast as possible so we always have work to do. One loop of data transfers
    // could be as short as 10ms (maximum update rate of the sensors) but will actually be longer.
    // No exact loop timing is guarateed, as we may have to skip cycles if requests are not
    // yet fulfilled. To save overhead, this function is directly called from the interrupt() routine
    // and not schedules as a task. In case of errors tasks are scheduled to do the reporting.
    void read_sensor();

    // This is the worker function being executed by the taskmanager.
    // It performs a loop with query_state indicating the cycle.
    virtual void run() {};

    // While the data acquisition from the sensor is done within the interrupt routine
    // for sending messages we use tasks.
    void report_quat_size_mismatch();
    void report_gyro_size_mismatch();
    void report_cycles_overrun();
    
    // destructor
    virtual ~MotionSensor() {};

    // port over which status messages are sent
    SenderPort status_out;

    // port over which angular data is sent out at requested rate
    StreamSender<DATA_IMU_AHRS> AHRS_out;
    
    // port over which gyro data is sent
    StreamSender<DATA_IMU_GYRO> GYRO_out;

private:
    
    // convert raw quaternion data into AHRS angles
    // computes heading, pitch, roll
    void convert_Quaternion(BNO055::sQuaData_t raw);
    
    float       heading;    // in degree (north=0, east=90)
    float       pitch;      // nose angle of attack in degree (up positive)
    float       roll;		// angle of rotation about the heading direction
                            // usual roll angle in horizontal flight
                            // lean angle (yaw) in hover

    float       gyr_x;      // rate of angular rotation [deg/s] about the forwad direction (roll right positive)
    float       gyr_y;      // rate of angular rotation [deg/s] about the wing axis (pitch up positive)
    float       gyr_z;      // rate of angular rotation [deg/s] about the belly axis (heading positive change or yaw right in hover)

    // variables for the sensor access
    
    BNO055                  *bno055;        // the IMU sensor
    uint8_t                 BNO055_I2C;     // BNO-055 slave address
    bool                    bno055_OK;      // sensor state
    
    // here are some flags indicating which work is done
    
    uint32_t    last_calib_check;           // the time when the last calibration check has beed done
    uint8_t     last_cal_state;
    
    int         query_state;    // which sensor communication is currently done
                                // this is counting run cycles 1..10 creating the 100 Hz update rate
    int         cycle_count;    // we count the number of cycles one loop actually takes.
    
};
