/*
    Definitions of the composed data types that can be used
    for messages and streams
*/

#pragma once

// all data types are assigned a signature that makes them recognizeable in the log files

#define DATA_IMU_AHRS_SIGNATURE 0xa0
#define DATA_IMU_GYRO_SIGNATURE 0xa1

// in earth-fixed coordinates
struct DATA_IMU_AHRS {
    float   attitude;       // angle of attack with respect to horizontal flight [deg]
                            // range  -180 ... +180 deg, positive up (hover is +90 deg)
    float   heading;        // with respect to magnetic north [deg], range 0 ... 360 deg
                            // nose direction for attitude=-90..+90 deg, else tail direction
    float   roll;           // roll angle about heading (roll in horizontal flight, yaw in hover)
                            // range -90 ... 90 deg, positive left
};

// in airframe-fixed coordinates (right, forward, up)
struct DATA_IMU_GYRO {
    float   nick;           // rate [deg/s] positive up
    float   yaw;            // rate [deg/s] positive left
    float   roll;           // rate [deg/s] positive left
};

