/*
    All modules communicate by messages.
    The communication can be driven by the sender (streaming)
    or the receiver (message requests). Usually the slower module initiates
    the transmission and determines te data rate.
    In all cases the transmission occurs between a sender and a receiver port.
    
    The modules shall be as independent as possible, so, usually they don't
    know the type of module they are communicating with. The ports are wired
    at runtime during system creation when all modules are created.

    All messages contain a sender information, a signature byte of the
    message type and a type-specific data block. This way in principle
    all ports can send and receive arbitrary types of messages.
    The date block is preceeded by its size in bytes and usually
    contains a strucure. The most common message types are defined here
    but modules could well define more types.
    Receiver ports may drop messages which are of not-handled types.
*/

#pragma once

#include <cstdint>
#include <string>

// message type IDs
# define MSG_TYPE_TEXT              1
# define MSG_TYPE_SYSTEM            2
# define MSG_TYPE_TELEMETRY         3
# define MSG_TYPE_GPS_POSITION     10
# define MSG_TYPE_IMU_GYRO         20
# define MSG_TYPE_IMU_ACCEL        21
# define MSG_TYPE_IMU_MAG          22
# define MSG_TYPE_IMU_AHRS         23

// ---------------------------------------------
// data structure of the different message types
// ---------------------------------------------

// general text message (may be another message serialized)
struct  MSG_TEXT {
    std::string text;
};

struct  MSG_SYSTEM {
    uint32_t time;
    uint8_t severity_level;
    std::string text;
};

struct  MSG_TELEMETRY {
    uint32_t time;
    std::string variable;
    std::string value;
};

struct  MSG_GPS_POSITION {
    double  latitude;       // degree north
    double  longitude;      // degree east
    float   altitude;       // meters above MSL
};

// in airframe-fixed coordinates (right, forward, up)
struct MSG_IMU_GYRO {
    float   nick;           // rate [deg/s] positive up
    float   roll;           // rate [deg/s] positive left
    float   yaw;            // rate [deg/s] positive left
};

// in airframe-fixed coordinates (right, forward, up)
struct MSG_IMU_ACCEL {
    float   right;          // acceleration [G]
    float   forward;        // acceleration [G]
    float   up;             // acceleration [G]
};

// in airframe-fixed coordinates (right, forward, up)
struct MSG_IMU_MAG {
    float   right;          // field Bx [Gs]
    float   forward;        // field By [Gs]
    float   up;             // field Bz [Gs]
};

// in earth-fixed coordinates
struct MSG_IMU_AHRS {
    float   attitude;       // angle of attack with respect to horizontal flight [deg]
                            // range  -180 ... +180 deg, positive up (hover is +90 deg)
    float   heading;        // with respect to magnetic north [deg], range 0 ... 360 deg
                            // nose direction for attitude=-90..+90 deg, else tail direction
    float   roll;           // roll angle about heading (roll in horizontal flight, yaw in hover)
                            // range -90 ... 90 deg, positive left
};

// status messages have a severity level information
#define MSG_LEVEL_FATALERROR            1
#define MSG_LEVEL_CRITICAL              3
#define MSG_LEVEL_MILESTONE             5
#define MSG_LEVEL_ERROR                 8
#define MSG_LEVEL_STATE_CHANGE         10
#define MSG_LEVEL_WARNING              12
#define MSG_LEVEL_IMPORTANTTELEMETRY   20
#define MSG_LEVEL_STATUSREPORT         30
#define MSG_LEVEL_TELEMETRY            50


class Message {
    public:
        // standard constructor
        // the data block is copied into newly allocated memory
        Message(
            std::string sender_module,
            uint8_t msg_type,
            uint16_t data_size,
            unsigned char *data
            );
        
        // copy constructor
        // create a clone of the given message
        Message(const Message& msg);
        
        // destructor
        ~Message();

        // generate a possibly compact array of bytes for transmission over
        // bandwidth-limited communication channels
        std::string byte_string();
                
        // Generate a string with a standardized format holding the message.
        // There is no CR/LF at the end of the string, a print routine has to add that if necessary.
        std::string print();
        
        // Generate a text message with all information but the sender id serialized
        Message as_text();
        
    private:   
        // the sender module of the message
        std::string sender_module;
        
        // the message type
        uint8_t msg_type;
        
        // the sie of the data structure
        uint16_t data_size;
        
        // pointer to the data block
        // the data block will be allocated/freed with the message
        // upon creation it is filled from the provided memory location
        // when data_size==0 this pointer is NULL - do not access
        unsigned char *data;
};

