// ************************************************************
// test of the BNO-055 IMU with I²C interface
// ************************************************************

#include "Arduino.h"

// # include "Wire.h"
// the I²C bus is accessed via the teensy4_i2c library by Richard Gemmell
// https://github.com/Richard-Gemmell/teensy4_i2c


// using a modified version of the DFRobot_BNO055 library
// communicating via i2c_t3 instead of Wire
// #include "DFRobot_BNO055.h"
#include "i2c_driver_wire.h"

// using a modified version of the DFRobot_BNO055 library
// communicating via i2c_t3 instead of Wire
#include "DFRobot_BNO055.h"

// ************************************************************
// data and pin definitions for the IMU
// ************************************************************

uint8_t BNO055_I2C = 0x28;              // BNO-055 Slave address

typedef DFRobot_BNO055_IIC    BNO;      // use abbreviation instead of full name
BNO     bno055(&Wire, BNO055_I2C);      // the device object
bool    bno055_OK = false;              // wether the BNO-055 is available and properly initialized

BNO::sAxisAnalog_t  sMagAnalog;         // magnetometer reading
BNO::sAxisAnalog_t  sGyrAnalog;         // gyro reading
BNO::sAxisAnalog_t  sGrvAnalog;         // gravity vector
BNO::sEulAnalog_t   sEulAnalog;         // euler angles
BNO::sQuaAnalog_t   sQuaAnalog;         // quaternion : float  w, x, y, z;

typedef struct {
    float   head, pitch, yaw;
} sHover_t;

BNO::sEulAnalog_t   AHRS;               // euler angles in normal flight (pitch<45deg)
sHover_t            HoverAHRS;          // euler angles for hover (pitch>45deg)

// ************************************************************
// setup routines
// ************************************************************

void setup_bno()
{
    Serial.println("bno055() setup");
    bno055_OK = true;
    // check ID registers
    uint8_t tmp = (uint8_t){0xff};
    bno055.readReg(0x00, &tmp, 1);
    Serial.print("bno055() chip ID : ");
    if (tmp != 0xA0) bno055_OK = false;
    Serial.println((int)tmp,HEX);
    bno055.readReg(0x01, &tmp, 1);
    Serial.print("bno055() ACC chip ID : ");
    if (tmp != 0xFB) bno055_OK = false;
    Serial.println((int)tmp,HEX);
    bno055.readReg(0x02, &tmp, 1);
    Serial.print("bno055() MAG chip ID : ");
    if (tmp != 0x32) bno055_OK = false;
    Serial.println((int)tmp,HEX);
    bno055.readReg(0x03, &tmp, 1);
    Serial.print("bno055() GYRO chip ID : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0x0F) bno055_OK = false;
    bno055.readReg(0x36, &tmp, 1);
    Serial.print("bno055() self test ST_RESULT : ");
    Serial.println((int)tmp,HEX);
    if (tmp != 0x0F) bno055_OK = false;
    
    // reset() is performed during the begin() procedure
    // remapping the axes is done inside the begin() method
    // airframe-fixed coordinates ==  x: forward (nose)  y: left (wing)  z: up
    // Thereafter the sensor is switched to NDOF fusion mode
    while(bno055.begin() != BNO::eStatusOK) {
        Serial.println("bno055.begin() failed - status ");
        Serial.println(bno055.lastOperateStatus);
        bno055_OK = false;
        delay(2000);
    }
    Serial.println("bno055.begin() success");

    bno055.readReg(0x41, &tmp, 1);
    Serial.print("bno055() axis remap config : ");
    Serial.println((int)tmp,HEX);
    bno055.readReg(0x42, &tmp, 1);
    Serial.print("bno055() axis sign config : ");
    Serial.println((int)tmp,HEX);
}

// gather IMU data (takes about 3-6 ms)
// included reading the quaternion up to 8ms
void read_IMU()
{
    uint32_t start = systick_millis_count;
    sMagAnalog = bno055.getAxis(BNO::eAxisMag);     // read geomagnetic
    sGyrAnalog = bno055.getAxis(BNO::eAxisGyr);     // read gyroscope
    sGrvAnalog = bno055.getAxis(BNO::eAxisGrv);     // read gravity vector
    sEulAnalog = bno055.getEul();                   // read euler angle
    sQuaAnalog = bno055.getQua();                   // read quaternion
    // transform the quaternion into the heading-pitch-roll angles
    double w = sQuaAnalog.w;
    double x = sQuaAnalog.x;
    double y = sQuaAnalog.y;
    double z = sQuaAnalog.z;
    double R33 = w*w - x*x - y*y + z*z ;
    double R32 = 2*(w*x + y*z);
    double R31 = 2*(x*z - w*y);
    double R11 = w*w + x*x - y*y - z*z ;
    double R21 = 2*(w*z + x*y);
    AHRS.roll = -(180.0/PI)*atan2(R32,R33);
    AHRS.pitch = (180.0/PI)*asin(-R31);
    AHRS.head = -(180.0/PI)*atan2(R21,R11);
    AHRS.head -= 90.0;
    if (AHRS.head<0) AHRS.head += 360.0;
    // heading-pitch-yaw angles for hover mode
    double R13 = 2*(w*y + x*z);
    double R23 = 2*(y*z - w*x);
    HoverAHRS.yaw = -(180.0/PI)*atan2(R32,-R31);
    HoverAHRS.pitch = (180.0/PI)*acos(R33);
    HoverAHRS.head = (180.0/PI)*atan2(R13,R23);
    HoverAHRS.head += 180.0;
    uint32_t stop = systick_millis_count;
    Serial.print("=== read_IMU === ");
    Serial.print(stop-start);
    Serial.println(" ms");
}

// print IMU data (takes <1ms)
void print_IMU()
{
    char line[80];
    uint32_t start = systick_millis_count;
    Serial.println();
    sprintf(line,"x=%7.4f y=%7.4f z=%7.4f",sMagAnalog.x, sMagAnalog.y, sMagAnalog.z);
    Serial.print("mag analog (uT)  : "); Serial.println(line);
    sprintf(line,"x=%7.4f y=%7.4f z=%7.4f",sGyrAnalog.x, sGyrAnalog.y, sGyrAnalog.z);
    Serial.print("gyro    (deg/s)  : "); Serial.println(line);
    sprintf(line,"x=%7.4f y=%7.4f z=%7.4f",sGrvAnalog.x, sGrvAnalog.y, sGrvAnalog.z);
    Serial.print("grv analog (mg)  : "); Serial.println(line);
    sprintf(line,"Euler (deg) :  h=%8.4f  p=%8.4f  r=%8.4f",
        sEulAnalog.head, sEulAnalog.pitch, sEulAnalog.roll);
    Serial.println(line);
    sprintf(line,"Quaternion  :  w=%8.4f  x=%8.4f  y=%8.4f  z=%8.4f",
        sQuaAnalog.w, sQuaAnalog.x, sQuaAnalog.y, sQuaAnalog.z);
    Serial.println(line);
    sprintf(line,"AHRS (deg)  :  h=%8.4f  p=%8.4f  r=%8.4f",
        AHRS.head, AHRS.pitch, AHRS.roll);
    Serial.println(line);
    Serial.print("========  analog data print end  ======== ");
    uint32_t stop = systick_millis_count;
    Serial.print(stop-start);
    Serial.println(" ms");
}

// ************************************************************
// main program
// ************************************************************

// The loop() method is called  over and over again as long as the board has power.
// The loop executes code that has been scheduled by the systick ISR.
// For every loop call only one scheduled routine is executed.
// So, the sequence of checks in the loop gives a priorization of the different tasks.

void loop()
{
    // gather IMU data
    read_IMU();

    // print IMU data
    print_IMU();
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
