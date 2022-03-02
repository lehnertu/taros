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

/*
    This is the base class for all messages.
    It defines all functionality a message must provide.
    It cannot be used by itself, only derived classes that implement the defined functionality.
*/
class Message {
    public:
        // standard constructor
        Message(std::string sender_module);
        
        // Generate a string with a standardized format holding the message.
        // There is no CR/LF at the end of the string, a print routine has to add that if necessary.
        // this must be overridden by derived classes
        virtual std::string serialize() = 0;
        
        // Generate a text message with all information but the sender id serialized
        // this must be overridden by derived classes
        // we cannot declare it here because the derived class is still abstract
        // virtual Message_Text as_text() = 0;
        
        // there is one single member that is required for all messages
        // the sender module of the message
        std::string m_sender_module;
};

/*
    These are general text messages.
*/
class Message_Text : public Message {
    public:
        // constructor
        Message_Text(
            std::string sender_module,
            std::string text);
        // override the serialization function
        virtual std::string serialize();
        virtual Message_Text as_text();
        // additional members
        std::string m_text;
};

/*
    These are system messages going to a logfile or to the downlink.
    They have an indication of how important the message is
    and at which time the message was sent.
    time : milliseconds since system start will be serialized as seconds with 1 ms resolution
*/
class Message_System : public Message {
    public:
        // constructor
        Message_System(
            std::string sender_module,
            uint32_t time,
            uint8_t severity_level,
            std::string text);
        // override the serialization function
        virtual std::string serialize();
        virtual Message_Text as_text();
        // additional members
        uint8_t m_severity_level;
        uint32_t m_time;
        std::string m_text;
};

/*
    These are telemetry messages for automatted evaluation.
    They contain exactly one data item (variable) of given name.
    they contain the time the message was sent.
    time : milliseconds since system start will be serialized as seconds with 1 ms resolution
*/
class Message_Telemetry : public Message {
    public:
        // constructor
        Message_Telemetry(
            std::string sender_id,
            uint32_t time,
            std::string variable,
            std::string value);
        // override the serialization function
        virtual std::string serialize();
        virtual Message_Text as_text();
        // additional members
        uint32_t m_time;
        std::string m_variable;
        std::string m_value;
};

/*
    These are messages sent by a GPS module containing the current position
*/
class Message_GPS_position : public Message {
    public:
        // constructor
        Message_GPS_position(
            std::string sender_id,
            double  latitude,       // degree north
            double  longitude,      // degree east
            float   altitude);      // meters above MSL
        // override the serialization function
        virtual std::string serialize();
        virtual Message_Text as_text();
        // additional members
        double  m_latitude;
        double  m_longitude;
        float   m_altitude;
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

