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

struct MESSAGE_GPS_POSITION {
    double  latitude;       // degree north
    double  longitude;      // degree east
    double  altitude;       // meters above MSL
};

struct MESSAGE_LOGGING {
    
};
