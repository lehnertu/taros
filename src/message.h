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
    double  altitude;       // meters above MSL
};

// How to print the different message types
// There is no CR/LF at the end of the string, a print routine has to add that if necessary
std::string serialize_text_message(MESSAGE_TEXT msg);
std::string serialize_telemetry_message(MESSAGE_TELEMETRY msg);
    

