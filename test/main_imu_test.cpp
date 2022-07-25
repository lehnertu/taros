// ************************************************************
// test of the BNO-055 IMU with I²C interface
// ************************************************************

#include "Arduino.h"

// using a modified version of the BNO055 library
// the I²C bus is accessed via the teensy4_i2c library by Richard Gemmell
// https://github.com/Richard-Gemmell/teensy4_i2c
#include "i2c_driver_wire.h"
#include "bno055.h"

// SD card access
#include <SD.h>
#include <SPI.h>

// ************************************************************
// data and pin definitions for the IMU
// ************************************************************

uint8_t BNO055_I2C = 0x28;              // BNO-055 Slave address

BNO055  bno055(&Wire, BNO055_I2C);      // the device object
bool    bno055_OK = false;              // wether the BNO-055 is available and properly initialized

BNO055::sAxisAnalog_t  sMagAnalog;          // magnetometer reading
BNO055::sAxisAnalog_t  sGyrAnalog;          // gyro reading
BNO055::sAxisAnalog_t  sGrvAnalog;          // gravity vector
BNO055::sQuaAnalog_t   Quat;                // quaternion : float  w, x, y, z;

typedef struct {
    float   head, pitch, yaw;
} sHover_t;

// ************************************************************
// setup routines
// ************************************************************

/*
    The sensor orientation on the board is:
        x:forward y:left z:up
    We want to use a coordinate system leading to the Tai Bryan angles:
        x:forward y:right z:down
    We have to invert the y and z axes and we have to flip x and y

    Then the rotations about the axis are:
        x: roll (positive to the right, zero is level wings, range +/- 180 deg)
        y: pitch (positive nose up, zero is level flight, range +/- 180 deg)
        z: heading (positive to the right, zero is north, range 0...360 deg)
*/

void setup_bno()
{
    // set fast mode I²C
    Wire.setClock(400000);
    Serial.println("bno055() setup");
    bno055_OK = true;
    // check ID registers
    uint8_t tmp = (uint8_t){0xff};
    bno055.readReg(0x00, &tmp, 1);
    Serial.print("bno055() chip ID : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0xA0) bno055_OK = false;
    bno055.readReg(0x36, &tmp, 1);
    Serial.print("bno055() self test ST_RESULT : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0x0F) bno055_OK = false;
    
    // reset() is performed during the begin() procedure
    // remapping the axes is done inside the begin() method
    // airframe-fixed coordinates ==  x: forward (nose)  y: left (wing)  z: up
    // Thereafter the sensor is switched to NDOF fusion mode
    while(bno055.begin() != BNO055::eStatusOK) {
        Serial.print("bno055.begin() failed - status ");
        Serial.println(bno055.lastOperateStatus);
        bno055_OK = false;
        delay(2000);
    }
    Serial.println("bno055.begin() success");

    // configure sensor
    // the sensor settings can only be altered while in non-fusion modes
    bno055.setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_CONFIG);
    delay(50);
    
    // axes remap P1
    bno055.setReg(BNO055::BNO055_AXIS_MAP_CONFIG_ADDR, 0, 0x24);
    // invert y and z axis signs
    bno055.setReg(BNO055::BNO055_AXIS_MAP_SIGN_ADDR, 0, 0x03);
    
    // The timing and bandwidth settings for the sensors have no effect.
    // They will overwritten by the sensor fusion algorithm when switching to NDOF mode.
    // In that mode the accelerometer and gyro will be updated at 100Hz
    // the magnetometer at 20 Hz for a combined fusion rate of 100 Hz
     
    // xxxxxxx0b : Accelerometer = m/s^2
    // xxxxxx1xb : Gyro = deg/s
    // xxxxx0xxb : Euler = Degrees
    // xxx0xxxxb : Temperature = Celsius
    // 1xxxxxxxb : sign of pitch : clockwise decreasing "Android"
    bno055.setReg(BNO055::BNO055_UNIT_SEL_ADDR, 0, 0x80);
    
    // accelerometer
    // xxxxxx01b : range +/- 4G
    // xxx100xxb : bandwidth 125 Hz
    // 000xxxxxb : normal operation
    bno055.setReg(BNO055::ACCEL_CONFIG_ADDR, 1, 0x11);
    
    // gyro
    // xxxxx001b : range +/- 1000 deg/s
    // xx010xxxb : bandwidth 116 Hz
    bno055.setReg(BNO055::GYR_CONFIG0_ADDR, 1, 0x11);
    // xxxxx000b : normal power
    bno055.setReg(BNO055::GYR_CONFIG1_ADDR, 1, 0x00);
    
    // magnetometer
    // xxxxx101b : 20 Hz
    // xxx11xxxb : high accuracy
    // x11xxxxxb : "forced" power mode
    bno055.setReg(BNO055::MAG_CONFIG_ADDR, 1, 0x7d);
    
    // switch to sensor fusion mode
    bno055.setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_NDOF);
    delay(50);

    // TODO:
    // external crystal ??
}

// gather IMU data (takes about 3-6 ms)
// included reading the quaternion up to 8ms
void read_IMU()
{
    uint32_t start = systick_millis_count;
    
    sMagAnalog = bno055.getMag();                       // read geomagnetic vector
    sGyrAnalog = bno055.getGyro();                      // read gyroscope
    sGrvAnalog = bno055.getGravity();                   // read gravity vector
    
    // Quat = bno055.getQuaternion();                      // read quaternion
    // For reading the quaternion data we use a non-blocking read method.
    char line[80];
    BNO055::sQuaData_t raw;
    uint32_t t0 = micros();
    bno055.NonBlockingRead_init(BNO055::BNO055_QUATERNION_DATA_W_LSB_ADDR);
    uint32_t t1 = micros();
    while (!bno055.NonBlockingRead_finished()) {};
    uint32_t t2 = micros();
    sprintf(line,"NonBlockingRead_init:%d wait:%d micros", t1-t0, t2-t1);
    Serial.println(line);
    // TODO: test for error
    t0 = micros();
    bno055.NonBlockingRead_request(sizeof(raw));
    t1 = micros();
    while (!bno055.NonBlockingRead_finished()) {};
    t2 = micros();
    sprintf(line,"NonBlockingRead_request:%d wait:%d micros", t1-t0, t2-t1);
    Serial.println(line);
    // TODO: test for error
    t0 = micros();
    uint8_t n_bytes = bno055.NonBlockingRead_available();
    bno055.NonBlockingRead_getData((uint8_t*) &raw, (uint8_t)sizeof(raw));
    t1 = micros();
    sprintf(line,"NonBlockingRead_getData:%d micros %d bytes", t1-t0, n_bytes);
    Serial.println(line);
    // scale : 1.0 = 2^14 lsb
    Quat.w = raw.w * 0.000061035;
    Quat.x = raw.x * 0.000061035;
    Quat.y = raw.y * 0.000061035;
    Quat.z = raw.z * 0.000061035;

    uint32_t stop = systick_millis_count;
    Serial.println();
    Serial.print("=== read_IMU === ");
    Serial.print(stop-start);
    Serial.println(" ms");
}

// calibration state
// when fully calibrated (0xFF) read and store calibration data
uint8_t calib;

Sd2Card card;

// save calibration data to SD card
// TODO: do we need to switch to calibration mode ?
void saveCalibration()
{
    // the address block 0x55 ... 0x6a contains
    // offsets for acc, mag, and gyro (3x 16-bit each)
    // radius acc, mag (16-bit each)
    uint8_t data[22];
    // switch to config mode
    bno055.setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_CONFIG);
    delay(10);
    // read all data as one block 22 byte
    bno055.readReg(BNO055::ACCEL_OFFSET_X_LSB_ADDR, data, 22);
    if (!card.init(SPI_HALF_SPEED, BUILTIN_SDCARD)) {
        Serial.println("SD card initialization failed.");
    } else {
        Serial.println("SD card found.");
        File dataFile = SD.open("BNO055_calibration.dat", FILE_WRITE);
        // if the file is available, write to it:
        if (dataFile) {
            // dataFile.println(dataString);
            // size_t write(const void* buf, size_t count)
            dataFile.write(data,sizeof(data));
            dataFile.close();
        } else {
            // if the file isn't open, pop up an error:
            Serial.println("error opening BNO055_calibration.dat");
        }
        Serial.println("file write done!");
    }
    delay(100);
    // switch back to sensor fusion mode
    bno055.setReg(BNO055::BNO055_OPR_MODE_ADDR, 0, BNO055::OPERATION_MODE_NDOF);
    delay(100);
}

// print IMU data (takes <1ms)
void print_IMU()
{
    char line[80];
    sprintf(line,"      mag        gravity    gyro");
    Serial.println(line);
    sprintf(line,"      [uT]       [m/s^2]    [deg/s]");
    Serial.println(line);
    sprintf(line,"x=    %8.4f   %8.4f   %8.4f",sMagAnalog.x, sGrvAnalog.x, sGyrAnalog.x);
    Serial.println(line);
    sprintf(line,"y=    %8.4f   %8.4f   %8.4f",sMagAnalog.y, sGrvAnalog.y, sGyrAnalog.y);
    Serial.println(line);
    sprintf(line,"z=    %8.4f   %8.4f   %8.4f",sMagAnalog.z, sGrvAnalog.z, sGyrAnalog.z);
    Serial.println(line);

    // the x-gyro gives the roll rate (to the right)
    // the y gyro gives the pitch rate (up)
    // the z gyro gives the yaw rate (heading change)
    Serial.println();
    sprintf(line,"Quaternion         : w=%8.4f x=%8.4f y=%8.4f z=%8.4f  q2=%8.4f",
        Quat.w, Quat.x, Quat.y, Quat.z,
        Quat.w*Quat.w + Quat.x*Quat.x + Quat.y*Quat.y + Quat.z*Quat.z
        );
    Serial.println(line);
    // transform the quaternion into the heading-pitch-roll angles
    double w = Quat.w;
    double x = Quat.x;
    double y = Quat.y;
    double z = Quat.z;
    // vec(d11, d21, d31) gives the direction of the x axis (nose) in the NED coordinate system
    double d11 = 2.0*w*z + 2.0*x*y;
    double d21 = w*w + x*x - y*y - z*z;
    double d31 = 2.0*w*y - 2.0*x*z;
    sprintf(line,"nose               :  N=%8.4f E=%8.4f down=%8.4f", d11, d21, d31);
    Serial.println(line);
    // vec(d12, d22, d32) gives the direction of the y axis (right wing) in the NED coordinate system
    double d12 = w*w - x*x + y*y - z*z;
    double d22 = -2.0*w*z + 2.0*x*y;
    double d32 = -2.0*w*x - 2.0*y*z;
    sprintf(line,"right wing         :  N=%8.4f E=%8.4f down=%8.4f", d12, d22, d32);
    Serial.println(line);
    // The heading is derived from the right wing vector projected into the N-E plane.
    // This gives reliable reading both in level flight and hover.
    // In inverted flight this is opposite to the flight path.
    // It is unreliable near knife-edge flight.
    // Heading is constant during a looping but jumps during a roll.
    double heading = (180.0/PI)*atan2(d12,-d22)+180.0;
    sprintf(line,"heading            :  %8.1f deg", heading);
    Serial.println(line);
    // heading vector in the plane
    double h_north = cos(PI/180.0*heading);
    double h_east = sin(PI/180.0*heading);
    sprintf(line,"heading vector     :  N=%8.4f E=%8.4f", h_north, h_east);
    Serial.println(line);
    // The pitch is computed from the nose angle in the plane given by the heading and down vectors
    double nose_forward = d11*h_north + d21*h_east;
    double nose_down = d31;
    double pitch = (180.0/PI)*atan2(-nose_down,nose_forward);
    sprintf(line,"pitch              :  %8.1f deg", pitch);
    Serial.println(line);
    // Roll is computed as the angle between the right wing in the plane given
    // by the heading+90deg vector and the down vector (i.e. the angle between the wing and the level plane).
    // This gives a continuous transition between the usual roll during forward flight
    // and the yaw angle needed during hover.
    // TODO : Its not a bug its a feature :
    // The heading flips when rolling inverted - as a result the roll angle is inverted, too.
    double w_north = -sin(PI/180.0*heading);
    double w_east = cos(PI/180.0*heading);
    sprintf(line,"wing vector        :  N=%8.4f E=%8.4f", w_north, w_east);
    Serial.println(line);
    double wing_right = d12*w_north + d22*w_east;
    double wing_down = d32;
    double roll = (180.0/PI)*atan2(wing_down,wing_right);
    sprintf(line,"roll/yaw           :  %8.1f deg", roll);
    Serial.println(line);
    // check the calibration status
    calib = bno055.getReg(BNO055::BNO055_CALIB_STAT_ADDR, 0);
    Serial.print("calibration status : ");
    Serial.println(calib,HEX);
    Serial.println("========  analog data print end  ======== ");
}

// ************************************************************
// main program
// ************************************************************

// The loop() method is called  over and over again as long as the board has power.
// The loop executes code that has been scheduled by the systick ISR.
// For every loop call only one scheduled routine is executed.
// So, the sequence of checks in the loop gives a priorization of the different tasks.

bool written_OK = false;

void loop()
{
    // gather IMU data
    read_IMU();

    // print IMU data
    print_IMU();

    // save calibration when ready
    if (!written_OK)
    {
        if (calib==(uint8_t)0xff)
        {
            Serial.println("\n  writing calibration data...\n");
            saveCalibration();
            written_OK = true;
        }
    }
};

extern "C" int main(void)
{

    // USB serial communication
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n\n  BNO-055 Test\n");

    Serial.println("  Initializing IMU ...\n");
    setup_bno();

    
    // loop forever
    while (true) {
        // the loop is the worker handling all scheduled executions
        loop();

        delay(1000);
        // allow other (system) checks (serial connections in particular)
        yield();
    };
    return 0; // Never reached.

}
