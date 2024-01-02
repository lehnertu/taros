#include <cstdio>

#include "base.h"
#include "global.h"
#include "motion.h"
#include "util.h"

// using a modified version of the BNO055 library
// the I²C bus is accessed via the teensy4_i2c library by Richard Gemmell
// https://github.com/Richard-Gemmell/teensy4_i2c
#include "i2c_driver_wire.h"
#include "bno055.h"

MotionSensor::MotionSensor(
    std::string name) : Module(name)
{
    query_state = 0;
    heading = 0.0;
    pitch = 0.0;
    roll = 0.0;
    gyr_x = 0.0;
    gyr_y = 0.0;
    gyr_z = 0.0;
    last_calib_check = 0;
    last_cal_state = 0;
    runlevel_= MODULE_RUNLEVEL_STOP;
}

void MotionSensor::setup()
{
    BNO055_I2C = 0x28;
    bno055 = new BNO055(&Wire, BNO055_I2C);
    
    // set fast mode I²C
    Wire.setClock(400000);
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "BNO-055 setup()") );
    bno055_OK = true;
    
    // check ID registers
    uint8_t tmp = (uint8_t){0xff};
    bno055->readReg(0x00, &tmp, 1);
    if (tmp != 0xA0)
    {
        bno055_OK = false;
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 wrong chip ID") );
    };
    bno055->readReg(0x36, &tmp, 1);
    if (tmp != 0x0F)
    {
        bno055_OK = false;
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 self-test failed.") );
    };    
    // reset() is performed during the begin() procedure
    // remapping the axes is done inside the begin() method
    // airframe-fixed coordinates ==  x: forward (nose)  y: left (wing)  z: up
    // Thereafter the sensor is switched to NDOF fusion mode
    while(bno055->begin() != BNO055::eStatusOK) {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 begin() failed.") );
        bno055_OK = false;
        delay(2000);
    }
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "BNO-055 begin() success.") );

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
    };
    // write calibration to the sensor
    if (data_OK)
    {
        bno055->setToPage(0);
        // switch to config mode
        bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_CONFIG);
        delay(50);
        bno055->writeReg(BNO055::ACCEL_OFFSET_X_LSB_ADDR, data, 22);
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "BNO-055 calibrated from file.") );
    }
    else
    {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "error reading BNO-055 calibration data file.") );
    };
    
    // switch to sensor fusion mode
    bno055->setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_NDOF);
    delay(100);

    // TODO: check the calibration status before switching to operation
    // system calib status must be 3 (bit 6 and 7 set)
    check_calibration();

    // we have to read it once, only then the non-blocking reads will work
    bno055->getQuaternion();

    // now the interrups becomes active
    runlevel_= MODULE_RUNLEVEL_SETUP_OK;

}

void MotionSensor::check_calibration()
{
    // TODO: only report calibration status when it has changed
    last_calib_check = FC_time_now();
    // system calib status must be 3 (bit 6 and 7 set)
    uint8_t cal = bno055->getCalibrationStatus();
    if (cal != last_cal_state)
    {
        last_cal_state = cal;
        std::string report("calibration status : ");
        report += hexbyte(cal);
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, report) );
    }
    if (cal>=(uint8_t)0xc0)
    {
        status_out.transmit(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
        runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    }    
}

// this is a transition version only
// as long as the sensor is not yet in MODULE_RUNLEVEL_OPERATIONAL
// we schedule tasks to check the calibration
// later on the old run() method takes over
void MotionSensor::interrupt()
{
    if ((runlevel_ >= MODULE_RUNLEVEL_SETUP_OK) && (runlevel_ < MODULE_RUNLEVEL_OPERATIONAL))
        if (FC_elapsed_millis(last_calib_check)>1000)
            schedule_task(this, std::bind(&MotionSensor::check_calibration, this));
}

void MotionSensor::report_quat_size_mismatch()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 quaternion data size mismatch.") );
}

void MotionSensor::report_gyro_size_mismatch()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_CRITICAL, "BNO-055 gyro data size mismatch.") );
}

void MotionSensor::report_cycles_overrun()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_WARNING, "IMU loop exceeding 10 cycles.") );
}

void MotionSensor::run()
{
    static BNO055::sQuaData_t raw = {0,0,0,0};
    static BNO055::sAxisData_t gyr = {0,0,0};
    switch (query_state)
    {
        case 0:
        {
            cycle_count++;
            // initiate a non-blocking read for the quaternion data
            bno055->NonBlockingRead_init(BNO055::BNO055_QUATERNION_DATA_W_LSB_ADDR);
            query_state++;
            break;
        };
        case 1:
        {
            cycle_count++;
            // the read should be finished by now
            // most often this takes another cycle
            if (bno055->NonBlockingRead_finished())
            {
                // request the data
                bno055->NonBlockingRead_request(sizeof(raw));
                // we only continue when the transaction has finished
                query_state++;
            };
            break;
        };
        case 2:
        {
            cycle_count++;
            // the request should be fulfilled by now
            // most often this takes one more cycle
            if (bno055->NonBlockingRead_finished())
            {
                uint8_t n_bytes = bno055->NonBlockingRead_available();
                if (n_bytes == sizeof(raw))
                    // copy the data from buffer
                    bno055->NonBlockingRead_getData((uint8_t*) &raw, (uint8_t)sizeof(raw));
                else
                    schedule_task(this, std::bind(&MotionSensor::report_quat_size_mismatch, this));
                // we only continue, if the request was fulfilled
                // TODO: this could lead to too many reports
                query_state++;
            };
            break;
        };
        case 3:
        {
            cycle_count++;
            // process the quaternion data
            convert_Quaternion(raw);
            // send out data messages
            DATA_IMU_AHRS data {
                .attitude = pitch,
                .heading = heading,
                .roll = roll };
            AHRS_out.transmit(data);
            query_state++;
            break;
        };
        case 4:
        {
            cycle_count++;
            // initiate a non-blocking read for the magnetometer data
            bno055->NonBlockingRead_init(BNO055::BNO055_GYRO_DATA_X_LSB_ADDR);
            query_state++;
            break;
        };
        case 5:
        {
            cycle_count++;
            // the read should be finished by now
            // most often this takes another cycle
            if (bno055->NonBlockingRead_finished())
            {
                // request the data
                bno055->NonBlockingRead_request(sizeof(gyr));
                // we only continue when the transaction has finished
                query_state++;
            };
            break;
        };
        case 6:
        {
            cycle_count++;
            // the request should be fulfilled by now
            // most often this takes one more cycle
            if (bno055->NonBlockingRead_finished())
            {
                uint8_t n_bytes = bno055->NonBlockingRead_available();
                if (n_bytes == sizeof(gyr))
                    // copy the data from buffer
                    bno055->NonBlockingRead_getData((uint8_t*) &gyr, (uint8_t)sizeof(gyr));
                else
                    schedule_task(this, std::bind(&MotionSensor::report_gyro_size_mismatch, this));
                // we only continue, if the request was fulfilled
                // TODO: this could lead to too many reports
                query_state++;
            };
            break;
        };
        case 7:
        {
            cycle_count++;
            // scale gyro data: 1 deg/s = 16 lsb
            gyr_x = 0.0625*gyr.x;
            gyr_y = 0.0625*gyr.y;
            gyr_z = 0.0625*gyr.z;
            DATA_IMU_GYRO data {
                .nick = gyr_y,
                .yaw = gyr_z,
                .roll = gyr_x };
            GYRO_out.transmit(data);
            query_state++;
            break;
        };
        // this case handles 8 and all above
        default:
        {
            // typically it takes 8 cycles to get here
            // when arriving here check if it took too long
            if (cycle_count>10)
                schedule_task(this, std::bind(&MotionSensor::report_cycles_overrun, this));
            query_state++;
            if (query_state>=10)
                // next loop
                query_state = 0;
            // reset the loop cycle count
            cycle_count=0;
            break;
        };
    };
}

// this is called within the interrupt service routine
// TODO: make local variabes static
void MotionSensor::convert_Quaternion(BNO055::sQuaData_t raw)
{
    // scale : 1.0 = 2^14 lsb
    float w = raw.w * 0.000061035;
    float x = raw.x * 0.000061035;
    float y = raw.y * 0.000061035;
    float z = raw.z * 0.000061035;
    // vec(d11, d21, d31) gives the direction of the x axis (nose) in the NED coordinate system
    double d11 = 2.0*w*z + 2.0*x*y;
    double d21 = w*w + x*x - y*y - z*z;
    double d31 = 2.0*w*y - 2.0*x*z;
    // vec(d12, d22, d32) gives the direction of the y axis (right wing) in the NED coordinate system
    double d12 = w*w - x*x + y*y - z*z;
    double d22 = -2.0*w*z + 2.0*x*y;
    double d32 = -2.0*w*x - 2.0*y*z;
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
