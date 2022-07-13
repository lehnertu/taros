/*!
 *  @file BNO055.h
 *
 *  This is a library for the BNO055 orientation sensor
 *  connected via I²C using a specialized library for the Teensy 4.x MCU
 *
 *  This library is derived from
 *   - the DFROBOT_BNO055 library by <@DFRobot Frank>
 *   - the Adafruit_BNO055 library by K.Townsend (Adafruit Industries)
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so.
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#ifndef BNO055_H
#define BNO055_H

// for usage with Arduino just change the includes to "Arduino.h" and "Wire.h"
#include "WProgram.h"
#include "i2c_driver_wire.h"

#ifndef PROGMEM
#define PROGMEM
#endif

class BNO055 {

// *******************************
//  BNO-055 registers
// *******************************

public :

    typedef enum {
        /* Page id register definition */
        BNO055_PAGE_ID_ADDR = 0X07,

        /* PAGE0 REGISTER DEFINITION START*/
        BNO055_CHIP_ID_ADDR = 0x00,
        BNO055_ACCEL_REV_ID_ADDR = 0x01,
        BNO055_MAG_REV_ID_ADDR = 0x02,
        BNO055_GYRO_REV_ID_ADDR = 0x03,
        BNO055_SW_REV_ID_LSB_ADDR = 0x04,
        BNO055_SW_REV_ID_MSB_ADDR = 0x05,
        BNO055_BL_REV_ID_ADDR = 0X06,

        /* Accel data register */
        BNO055_ACCEL_DATA_X_LSB_ADDR = 0X08,
        BNO055_ACCEL_DATA_X_MSB_ADDR = 0X09,
        BNO055_ACCEL_DATA_Y_LSB_ADDR = 0X0A,
        BNO055_ACCEL_DATA_Y_MSB_ADDR = 0X0B,
        BNO055_ACCEL_DATA_Z_LSB_ADDR = 0X0C,
        BNO055_ACCEL_DATA_Z_MSB_ADDR = 0X0D,

        /* Mag data register */
        BNO055_MAG_DATA_X_LSB_ADDR = 0X0E,
        BNO055_MAG_DATA_X_MSB_ADDR = 0X0F,
        BNO055_MAG_DATA_Y_LSB_ADDR = 0X10,
        BNO055_MAG_DATA_Y_MSB_ADDR = 0X11,
        BNO055_MAG_DATA_Z_LSB_ADDR = 0X12,
        BNO055_MAG_DATA_Z_MSB_ADDR = 0X13,

        /* Gyro data registers */
        BNO055_GYRO_DATA_X_LSB_ADDR = 0X14,
        BNO055_GYRO_DATA_X_MSB_ADDR = 0X15,
        BNO055_GYRO_DATA_Y_LSB_ADDR = 0X16,
        BNO055_GYRO_DATA_Y_MSB_ADDR = 0X17,
        BNO055_GYRO_DATA_Z_LSB_ADDR = 0X18,
        BNO055_GYRO_DATA_Z_MSB_ADDR = 0X19,

        /* Euler data registers */
        BNO055_EULER_H_LSB_ADDR = 0X1A,
        BNO055_EULER_H_MSB_ADDR = 0X1B,
        BNO055_EULER_R_LSB_ADDR = 0X1C,
        BNO055_EULER_R_MSB_ADDR = 0X1D,
        BNO055_EULER_P_LSB_ADDR = 0X1E,
        BNO055_EULER_P_MSB_ADDR = 0X1F,

        /* Quaternion data registers */
        BNO055_QUATERNION_DATA_W_LSB_ADDR = 0X20,
        BNO055_QUATERNION_DATA_W_MSB_ADDR = 0X21,
        BNO055_QUATERNION_DATA_X_LSB_ADDR = 0X22,
        BNO055_QUATERNION_DATA_X_MSB_ADDR = 0X23,
        BNO055_QUATERNION_DATA_Y_LSB_ADDR = 0X24,
        BNO055_QUATERNION_DATA_Y_MSB_ADDR = 0X25,
        BNO055_QUATERNION_DATA_Z_LSB_ADDR = 0X26,
        BNO055_QUATERNION_DATA_Z_MSB_ADDR = 0X27,

        /* Linear acceleration data registers */
        BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR = 0X28,
        BNO055_LINEAR_ACCEL_DATA_X_MSB_ADDR = 0X29,
        BNO055_LINEAR_ACCEL_DATA_Y_LSB_ADDR = 0X2A,
        BNO055_LINEAR_ACCEL_DATA_Y_MSB_ADDR = 0X2B,
        BNO055_LINEAR_ACCEL_DATA_Z_LSB_ADDR = 0X2C,
        BNO055_LINEAR_ACCEL_DATA_Z_MSB_ADDR = 0X2D,

        /* Gravity data registers */
        BNO055_GRAVITY_DATA_X_LSB_ADDR = 0X2E,
        BNO055_GRAVITY_DATA_X_MSB_ADDR = 0X2F,
        BNO055_GRAVITY_DATA_Y_LSB_ADDR = 0X30,
        BNO055_GRAVITY_DATA_Y_MSB_ADDR = 0X31,
        BNO055_GRAVITY_DATA_Z_LSB_ADDR = 0X32,
        BNO055_GRAVITY_DATA_Z_MSB_ADDR = 0X33,

        /* Temperature data register */
        BNO055_TEMP_ADDR = 0X34,

        /* Status registers */
        BNO055_CALIB_STAT_ADDR = 0X35,
        BNO055_SELFTEST_RESULT_ADDR = 0X36,
        BNO055_INTR_STAT_ADDR = 0X37,

        BNO055_SYS_CLK_STAT_ADDR = 0X38,
        BNO055_SYS_STAT_ADDR = 0X39,
        BNO055_SYS_ERR_ADDR = 0X3A,

        /* Unit selection register */
        BNO055_UNIT_SEL_ADDR = 0X3B,

        /* Mode registers */
        BNO055_OPR_MODE_ADDR = 0X3D,
        BNO055_PWR_MODE_ADDR = 0X3E,

        BNO055_SYS_TRIGGER_ADDR = 0X3F,
        BNO055_TEMP_SOURCE_ADDR = 0X40,

        /* Axis remap registers */
        BNO055_AXIS_MAP_CONFIG_ADDR = 0X41,
        BNO055_AXIS_MAP_SIGN_ADDR = 0X42,

        /* SIC registers */
        BNO055_SIC_MATRIX_0_LSB_ADDR = 0X43,
        BNO055_SIC_MATRIX_0_MSB_ADDR = 0X44,
        BNO055_SIC_MATRIX_1_LSB_ADDR = 0X45,
        BNO055_SIC_MATRIX_1_MSB_ADDR = 0X46,
        BNO055_SIC_MATRIX_2_LSB_ADDR = 0X47,
        BNO055_SIC_MATRIX_2_MSB_ADDR = 0X48,
        BNO055_SIC_MATRIX_3_LSB_ADDR = 0X49,
        BNO055_SIC_MATRIX_3_MSB_ADDR = 0X4A,
        BNO055_SIC_MATRIX_4_LSB_ADDR = 0X4B,
        BNO055_SIC_MATRIX_4_MSB_ADDR = 0X4C,
        BNO055_SIC_MATRIX_5_LSB_ADDR = 0X4D,
        BNO055_SIC_MATRIX_5_MSB_ADDR = 0X4E,
        BNO055_SIC_MATRIX_6_LSB_ADDR = 0X4F,
        BNO055_SIC_MATRIX_6_MSB_ADDR = 0X50,
        BNO055_SIC_MATRIX_7_LSB_ADDR = 0X51,
        BNO055_SIC_MATRIX_7_MSB_ADDR = 0X52,
        BNO055_SIC_MATRIX_8_LSB_ADDR = 0X53,
        BNO055_SIC_MATRIX_8_MSB_ADDR = 0X54,

        /* Accelerometer Offset registers */
        ACCEL_OFFSET_X_LSB_ADDR = 0X55,
        ACCEL_OFFSET_X_MSB_ADDR = 0X56,
        ACCEL_OFFSET_Y_LSB_ADDR = 0X57,
        ACCEL_OFFSET_Y_MSB_ADDR = 0X58,
        ACCEL_OFFSET_Z_LSB_ADDR = 0X59,
        ACCEL_OFFSET_Z_MSB_ADDR = 0X5A,

        /* Magnetometer Offset registers */
        MAG_OFFSET_X_LSB_ADDR = 0X5B,
        MAG_OFFSET_X_MSB_ADDR = 0X5C,
        MAG_OFFSET_Y_LSB_ADDR = 0X5D,
        MAG_OFFSET_Y_MSB_ADDR = 0X5E,
        MAG_OFFSET_Z_LSB_ADDR = 0X5F,
        MAG_OFFSET_Z_MSB_ADDR = 0X60,

        /* Gyroscope Offset register s*/
        GYRO_OFFSET_X_LSB_ADDR = 0X61,
        GYRO_OFFSET_X_MSB_ADDR = 0X62,
        GYRO_OFFSET_Y_LSB_ADDR = 0X63,
        GYRO_OFFSET_Y_MSB_ADDR = 0X64,
        GYRO_OFFSET_Z_LSB_ADDR = 0X65,
        GYRO_OFFSET_Z_MSB_ADDR = 0X66,

        /* Radius registers */
        ACCEL_RADIUS_LSB_ADDR = 0X67,
        ACCEL_RADIUS_MSB_ADDR = 0X68,
        MAG_RADIUS_LSB_ADDR = 0X69,
        MAG_RADIUS_MSB_ADDR = 0X6A
    } BNO055_reg_t;

    typedef enum {
        ACCEL_CONFIG_ADDR = 0x08,
        MAG_CONFIG_ADDR = 0x09,
        GYR_CONFIG0_ADDR = 0x0a,
        GYR_CONFIG1_ADDR = 0x0b,
        ACC_SLEEP_CONFIG_ADDR = 0x0c,
        GYR_SLEEP_CONFIG_ADDR = 0x0d,
        INTERRUPT_MASK_ADDR = 0x0f
    } BNO055_page1_reg_t;

    // BNO055 power settings
    typedef enum {
        POWER_MODE_NORMAL = 0X00,
        POWER_MODE_LOWPOWER = 0X01,
        POWER_MODE_SUSPEND = 0X02
    } BNO055_powermode_t;

    // Operation mode settings
    typedef enum {
        OPERATION_MODE_CONFIG = 0X00,
        OPERATION_MODE_ACCONLY = 0X01,
        OPERATION_MODE_MAGONLY = 0X02,
        OPERATION_MODE_GYRONLY = 0X03,
        OPERATION_MODE_ACCMAG = 0X04,
        OPERATION_MODE_ACCGYRO = 0X05,
        OPERATION_MODE_MAGGYRO = 0X06,
        OPERATION_MODE_AMG = 0X07,
        OPERATION_MODE_IMUPLUS = 0X08,
        OPERATION_MODE_COMPASS = 0X09,
        OPERATION_MODE_M4G = 0X0A,
        OPERATION_MODE_NDOF_FMC_OFF = 0X0B,
        OPERATION_MODE_NDOF = 0X0C
    } BNO055_opmode_t;
    
// *******************************
//  data structures
// *******************************

public:

    // status information returned by sensor access functions
    typedef enum {
        eStatusOK,                      // everything OK
        eStatusErr,                     // unknow error
        eStatusErrDeviceNotDetect,      // device not detected
        eStatusErrDeviceReadyTimeOut,   // device ready time out
        eStatusErrDeviceStatus,         // device internal status error
        eStatusErrParameter             // function parameter error
    } eStatus_t;

    // raw vector data structure
    typedef struct {
        int16_t   x, y, z;
    } sAxisData_t;

    // float vector data structure
    typedef struct {
        float   x, y, z;
    } sAxisAnalog_t;

    // raw quaternion data structure
    typedef struct {
        int16_t   w, x, y, z;
    } sQuaData_t;

    // float quaternion data structure
    typedef struct {
        float   w, x, y, z;
    } sQuaAnalog_t;

// *******************************
//  member functions
// *******************************

public:

    // constructor
    // define connection to module on interface pWire address addr
    BNO055(TwoWire *pWire, uint8_t addr);

    // initalize the sensor
    // return Sensor status
    eStatus_t begin();

    // read the gyro rates in deg/s
    sAxisAnalog_t getGyro();

    // read the magnetic field in µT
    sAxisAnalog_t getMag();

    // read the gravity vector in m/s²
    sAxisAnalog_t getGravity();
    
    // read the quaternion orientation
    sQuaAnalog_t getQuaternion();
    
// *******************************
//  internal use member functions
// *******************************

    // switch register pages - valid 0 or 1
    void setToPage(uint8_t pageId);
    
    // read/write a single byte register from a certain page
    // the page is switched when necessary and remains that way
    uint8_t getReg(uint8_t reg, uint8_t pageId);
    void setReg(uint8_t reg, uint8_t pageId, uint8_t val);
    
    // reading and writing registers on the sensor hardware
    // correct page is assumed
    void readReg(uint8_t reg, uint8_t *pBuf, uint8_t len);
    void writeReg(uint8_t reg, uint8_t *pBuf, uint8_t len);
    
// *******************************************
//  synchronous non-blocking data transmission
// *******************************************

    // To stay within the timing conatraints posed by the 1ms frame cycle
    // we have to decompose reading IMU data into 3 non-blocking steps
    // with the adress transmission and the data transmission
    // happening interrupt-driven in between
    
    // It is important that no other module accesses the same bus
    // this could interfere during the idle times between request and data handling
    
    // Step 1 : Initiate sending the register address to the slave (yield after this step)
    void NonBlockingRead_init(uint8_t reg);
    // Step 2 : check for having finished the address transmission (should take well below 1ms)
    bool NonBlockingRead_finished();
    // Step 3 : Initiate receiving the data (yield after this step)
    void NonBlockingRead_request(uint8_t len);
    // Step 4 : check if finished (6 or 8 bytes should take below 1ms)
    // Step 5 : get the data
    uint8_t NonBlockingRead_available();
    void NonBlockingRead_getData(uint8_t *pBuf, uint8_t len);

// *******************************
//  variables
// *******************************

public:

    eStatus_t   lastOperateStatus;
    
protected:

    uint8_t   _currentPage;
    TwoWire   *_pWire;          // the I²C bus handler
    uint8_t   _addr;            // the bus address
    
};

#endif
