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
    Quat = {0.0, 0.0, 0.0, 0.0};
    heading = 0.0;
    pitch = 0.0;
    roll = 0.0;
    gyr_x = 0.0;
    gyr_y = 0.0;
    gyr_z = 0.0;
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
    
    // TODO:
    // external crystal ??

    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "BNO-055 initialized.") );

    // read calibration data from file
    bool data_OK = false;
    uint8_t data[22];
    if (SD_card_OK and SD.exists("BNO055_calibration.dat"))
    {
        File dataFile = SD.open("BNO055_calibration.dat", FILE_READ);
        if (dataFile)
        {
            size_t numbytes = dataFile.read(data, 22);
            dataFile.close();
            if (numbytes==22) data_OK = true;
        };
    }
    else
    {
    }
    // write calibration to the sensor
    if (data_OK)
    {
        bno055->setToPage(0);
        // switch to config mode
        bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_CONFIG);
        delay(50);
        bno055->writeReg(BNO055::ACCEL_OFFSET_X_LSB_ADDR, data, 22);
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "BNO-055 calibrated.") );
    }
    else
    {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "error reading BNO-055 calibration data file.") );
    };
    
    // switch to sensor fusion mode
    bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_NDOF);
    delay(50);

    last_update = FC_time_now();
    flag_update_pending = false;
    
    
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );

    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

bool MotionSensor::have_work()
{
    if (runlevel_ == MODULE_RUNLEVEL_OPERATIONAL)
    {
        bool running = (query_state != 0);
        // TODO: we may need a timeout here
        // we decrease the rate for testing
        if (FC_elapsed_millis(last_update)>=200)
        // if (FC_elapsed_millis(last_update)>=10)
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
        return (flag_update_pending || running);
    }
    else
    {
        return false;
    };
}

void MotionSensor::run()
{
    BNO055::sQuaData_t raw = {0,0,0,0};
    switch (query_state)
    {
        case 1:
        {
            // initiate a non-blocking read for the quaternion data
            bno055->NonBlockingRead_init(BNO055::BNO055_QUATERNION_DATA_W_LSB_ADDR);
            query_state = 2;
            break;
        };
        case 2:
        {
            // the read should be finished by now
            if (bno055->NonBlockingRead_finished())
            {
                // request the data
                bno055->NonBlockingRead_request(sizeof(raw));
            }
            else
            {
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 read timeout.") );
            };
            query_state = 3;
            break;
        };
        case 3:
        {
            // the request should be fulfilled by now
            if (bno055->NonBlockingRead_finished())
            {
                uint8_t n_bytes = bno055->NonBlockingRead_available();
                if (n_bytes == sizeof(raw))
                    bno055->NonBlockingRead_getData((uint8_t*) &raw, (uint8_t)sizeof(raw));
                else
                    status_out.transmit(
                        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 data size mismatch.") );
            }
            else
            {
                status_out.transmit(
                    Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 data request timeout.") );
            };
            query_state = 4;
            break;
        };
        case 4:
        {
            // process the quaternion data
            convert_Quaternion(raw);
            // send out data messages
            MSG_DATA_IMU_AHRS data {
                .attitude = pitch,
                .heading = heading,
                .roll = roll };
            Message msg = Message(id, MSG_TYPE_IMU_AHRS, sizeof(data), (void*)(&data) );
            AHRS_out.transmit(msg);
            // ready for the next query
            query_state = 0;
            flag_update_pending = false;
            break;
        };
        default:
        {
            // we should never get here
        };
    };
}

void MotionSensor::convert_Quaternion(BNO055::sQuaData_t raw)
{
    char line[80];
    sprintf(line,"Quaternion         : w=%d x=%d y=%d z=%d",
        raw.w, raw.x, raw.y, raw.z);
    Serial.println(line);
    // scale : 1.0 = 2^14 lsb
    float w = raw.w * 0.000061035;
    float x = raw.x * 0.000061035;
    float y = raw.y * 0.000061035;
    float z = raw.z * 0.000061035;
    Quat = {w,x,y,z};
    // vec(d11, d21, d31) gives the direction of the x axis (nose) in the NED coordinate system
    float d11 = - w*w - x*x + y*y + z*z;
    float d21 = 2.0*w*z + 2.0*x*y;
    float d31 = 2.0*w*y - 2.0*x*z;
    // vec(d12, d22, d32) gives the direction of the y axis (right wing) in the NED coordinate system
    float d12 = 2.0*w*z - 2.0*x*y;
    float d22 = w*w - x*x + y*y - z*z;
    float d32 = -2.0*w*x - 2.0*y*z;
    // The heading is derived from the right wing vector projected into the N-E plane.
    // This gives reliable reading both in level flight and hover.
    // In inverted flight this is opposite to the flight path.
    // It is unreliable near knife-edge flight.
    // Heading is constant during a looping but jumps during a roll.
    heading = (180.0/PI)*atan2(d12,-d22)+180.0;
    // heading vector in the plane
    float h_north = cos(PI/180.0*heading);
    float h_east = sin(PI/180.0*heading);
    // The pitch is computed from the nose angle in the plane given by the heading and down vectors
    float nose_forward = d11*h_north + d21*h_east;
    float nose_down = d31;
    pitch = (180.0/PI)*atan2(-nose_down,nose_forward);
    // Roll is computed as the angle between the right wing in the plane given
    // by the heading+90deg vector and the down vector (i.e. the angle between the wing and the level plane).
    // This gives a continuous transition between the usual roll during forward flight
    // and the yaw angle needed during hover.
    // TODO : Its not a bug its a feature :
    // The heading flips when rolling inverted - as a result the roll angle is inverted, too.
    float w_north = -sin(PI/180.0*heading);
    float w_east = cos(PI/180.0*heading);
    float wing_right = d12*w_north + d22*w_east;
    float wing_down = d32;
    roll = (180.0/PI)*atan2(wing_down,wing_right);
}
