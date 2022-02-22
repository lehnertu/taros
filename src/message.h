/*
    All modules communicate by messages.
    The communication can be driven by the sender (streaming)
    or the receiver (message requests). Usually the slower module initiates
    the transmission and determines te data rate.
    In all cases the transmission occurs between a sender and a receiver port.
    
    The modules shall be as independent as possible, so, usually they don't
    know the type of module they are communicating with. The ports are wired
    at runtime during system creation when all modules are created.
    
    The messages are typed, so a compile-time check of the compatibility
    of sender and receiver ports is possible. Every module can define it's
    own message types. This, however, required that modules import the
    headers of the other modules they want to communicate with.
    To avoid that, many message types are predefined.
    Modules can use those without knowing the sender/receiver in advance.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#define MSG_LEVEL_FATALERROR 1
#define MSG_LEVEL_CRITICAL 3
#define MSG_LEVEL_MILESTONE 5
#define MSG_LEVEL_ERROR 8
#define MSG_LEVEL_STATE_CHANGE 10
#define MSG_LEVEL_WARNING 12
#define MSG_LEVEL_IMPORTANTTELEMETRY 20
#define MSG_LEVEL_STATUSREPORT 30
#define MSG_LEVEL_TELEMETRY 50

struct MESSAGE_SYSTEM {
    std::string sender_module;
    uint8_t severity_level;
    std::string text;
};

struct MESSAGE_TEXT {
    std::string sender_module;
    std::string text;
};

struct MESSAGE_TELEMETRY {
    std::string sender_module;
    std::string variable;
    std::string value;
};

struct MESSAGE_GPS_POSITION {
    double  latitude;       // degree north
    double  longitude;      // degree east
    float   altitude;       // meters above MSL
};

// in airframe-fixed coordinates (right, forward, up)
struct MESSAGE_IMU_GYRO {
    float   nick;           // rate [deg/s] positive up
    float   roll;           // rate [deg/s] positive left
    float   yaw;            // rate [deg/s] positive left
};

// in airframe-fixed coordinates (right, forward, up)
struct MESSAGE_IMU_ACCEL {
    float   right;          // acceleration [G]
    float   forward;        // acceleration [G]
    float   up;             // acceleration [G]
};

// in airframe-fixed coordinates (right, forward, up)
struct MESSAGE_IMU_MAG {
    float   right;          // field Bx [Gs]
    float   forward;        // field By [Gs]
    float   up;             // field Bz [Gs]
};

// in earth-fixed coordinates
struct MESSAGE_IMU_AHRS {
    float   attitude;       // angle of attack with respect to horizontal flight [deg]
                            // range  -180 ... +180 deg, positive up (hover is +90 deg)
    float   heading;        // with respect to magnetic north [deg], range 0 ... 360 deg
                            // nose direction for attitude=-90..+90 deg, else tail direction
    float   roll;           // roll angle about heading (roll in horizontal flight, yaw in hover)
                            // range -90 ... 90 deg, positive left
};

// How to print the different message types
// There is no CR/LF at the end of the string, a print routine has to add that if necessary
template<typename msg_type>
std::string serialize_message(msg_type msg);
    

