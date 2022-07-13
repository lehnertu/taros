#include <cstdio>

#include "global.h"
#include "motion.h"

// using a modified version of the BNO055 library
// the I²C bus is accessed via the teensy4_i2c library by Richard Gemmell
// https://github.com/Richard-Gemmell/teensy4_i2c
#include "i2c_driver_wire.h"
#include "bno055.h"

MotionSensor::MotionSensor(
    std::string name)
{
    // copy the name
    id = name;
    query_state = 0;
    runlevel_= MODULE_RUNLEVEL_STOP;
}

void MotionSensor::setup()
{
    BNO055_I2C = 0x28;
    bno055 = new BNO055(&Wire, BNO055_I2C);
    
    // set fast mode I²C
    Wire.setClock(400000);
    Serial.println("bno055() setup");
    bno055_OK = true;
    
    // check ID registers
    uint8_t tmp = (uint8_t){0xff};
    bno055->readReg(0x00, &tmp, 1);
    Serial.print("bno055() chip ID : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0xA0) bno055_OK = false;
    bno055->readReg(0x36, &tmp, 1);
    Serial.print("bno055() self test ST_RESULT : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0x0F) bno055_OK = false;
    
    // reset() is performed during the begin() procedure
    // remapping the axes is done inside the begin() method
    // airframe-fixed coordinates ==  x: forward (nose)  y: left (wing)  z: up
    // Thereafter the sensor is switched to NDOF fusion mode
    while(bno055->begin() != BNO055::eStatusOK) {
        Serial.print("bno055.begin() failed - status ");
        Serial.println(bno055->lastOperateStatus);
        bno055_OK = false;
        delay(2000);
    }
    Serial.println("bno055.begin() success");

    // configure sensor
    // the sensor settings can only be altered while in non-fusion modes
    bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_CONFIG);
    delay(50);
    
    // axes remap P1
    bno055->setReg(BNO055::BNO055_AXIS_MAP_CONFIG_ADDR, 0, 0x24);
    // invert y and z axis signs
    bno055->setReg(BNO055::BNO055_AXIS_MAP_SIGN_ADDR, 0, 0x03);
    
    // The timing and bandwidth settings for the sensors have no effect.
    // They will overwritten by the sensor fusion algorithm when switching to NDOF mode.
    // In that mode the accelerometer and gyro will be updated at 100Hz
    // the magnetometer at 20 Hz for a combined fusion rate of 100 Hz
     
    // xxxxxxx0b : Accelerometer = m/s^2
    // xxxxxx1xb : Gyro = deg/s
    // xxxxx0xxb : Euler = Degrees
    // xxx0xxxxb : Temperature = Celsius
    // 1xxxxxxxb : sign of pitch : clockwise decreasing "Android"
    bno055->setReg(BNO055::BNO055_UNIT_SEL_ADDR, 0, 0x80);
    
    // accelerometer
    // xxxxxx01b : range +/- 4G
    // xxx100xxb : bandwidth 125 Hz
    // 000xxxxxb : normal operation
    bno055->setReg(BNO055::ACCEL_CONFIG_ADDR, 1, 0x11);
    
    // gyro
    // xxxxx001b : range +/- 1000 deg/s
    // xx010xxxb : bandwidth 116 Hz
    bno055->setReg(BNO055::GYR_CONFIG0_ADDR, 1, 0x11);
    // xxxxx000b : normal power
    bno055->setReg(BNO055::GYR_CONFIG1_ADDR, 1, 0x00);
    
    // magnetometer
    // xxxxx101b : 20 Hz
    // xxx11xxxb : high accuracy
    // x11xxxxxb : "forced" power mode
    bno055->setReg(BNO055::MAG_CONFIG_ADDR, 1, 0x7d);
    
    // switch to sensor fusion mode
    bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_NDOF);
    delay(50);

    // TODO:
    // external crystal ??

    last_update = FC_time_now();
    flag_update_pending = false;
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

bool MotionSensor::have_work()
{
    // TODO: we may need a timeout here
    if (FC_elapsed_millis(last_update)>=10)
    {
        // if the last query is completed
        if (query_state == 0)
        {
            // next query
            query_state = 1;
            last_update = FC_time_now();
            flag_update_pending = true;
        }
    }
    else
    {
        // wait
        flag_update_pending = false;
    };
    return (flag_update_pending);
}

void MotionSensor::run()
{
    switch (query_state)
    {
        case 1:
        {
            query_state = 2;
            break;
        };
        case 2:
        {
            query_state = 3;
            break;
        };
        case 3:
        {
            // ready for the next query
            query_state = 0;
            break;
        };
        default:
        {
            // we should never get here
        };
    };
}

