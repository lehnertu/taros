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

/*
    All messages carry a type information.
    This is a 16-bit integer value which ist also transmitted over
    the ground communication link as a message signature (upper 10 bit == 0xCC80).
    That gives room for 63 message types.
    
    The body of the message is just a blob of binary data.
    This type information determines, what type of data we have to expect.
*/
#define MSG_TYPE_ABSTRACT       0xcc80
#define MSG_TYPE_SYSTEM         0xcc81
#define MSG_TYPE_TEXT           0xcc82
#define MSG_TYPE_TELEMETRY      0xcc83
#define MSG_TYPE_GPS_POSITION   0xcc84
#define MSG_TYPE_SERVO          0xcc85
/*
    All messages have a data body which has to be interpreted depending on the message type.
    These data bodies are structs declared here.
    
    All strings within the message are declared with their number of charcters
    as type TextSize. This implies that the corresponding number of characters
    follows afer the struct in the data blob (in the sequence the strings are listed).
    The strings have no termination and may themselves contain arbitrary binary data.
    
    The structs will be 32-bit aligned in memory and take correspondingly more space.
    Alternatively define them as struct __attribute__ ((packed))
*/
using TextSize = uint8_t;

struct MSG_DATA_SYSTEM {
    uint8_t     severity_level;
    uint32_t    time;
    TextSize    text;
};

// system messages carry a severity level information
#define MSG_LEVEL_FATALERROR 1
#define MSG_LEVEL_CRITICAL 3
#define MSG_LEVEL_MILESTONE 5
#define MSG_LEVEL_ERROR 8
#define MSG_LEVEL_STATE_CHANGE 10
#define MSG_LEVEL_WARNING 12
#define MSG_LEVEL_STATUSREPORT 30

struct MSG_DATA_TEXT {
    TextSize    text;
};

struct MSG_DATA_TELEMETRY {
    uint32_t    time;
    TextSize    variable;
    TextSize    value;
};

struct MSG_DATA_GPS_POSITION {
    double latitude;
    double longitude;
    float altitude;
};

#define NUM_SERVO_CHANNELS 8
struct MSG_DATA_SERVO {
    short int pos[NUM_SERVO_CHANNELS];
};

/*
    This message declares a telemetry vaiable.
    The combination of sender and variable name can be replaced
    by the hash in future data value transmissions.
    The hash also defines the data size and variable types
*/
struct MSG_TELEMETRY_VARIABLE {
    uint16_t    hash;
    TextSize    variable;
};

/*
    for all data messages no sender information (that is encoded in the hash)
    and no size is transmitted (that is encoded in the MSG_TYPE)
    just the timestamp and binary data
*/
#define MSG_TYPE_DATA_INT16     0xcc90
#define MSG_TYPE_DATA_FLOAT     0xcc98
#define MSG_TYPE_DATA_DOUBLE    0xcc99
#define MSG_TYPE_DATA_GPS       0xcca0      // double latitude, double longitude, float altitude
#define MSG_TYPE_IMU_AHRS       0xcca1      // float attitude, heading, roll
#define MSG_TYPE_IMU_GYRO       0xcca2      // float nick, yaw, roll

/*
    This is a message the can be sent and received in between modules.
    It holds information about the sender module and the size of the transmitted data block.
    The type information encodes which struct to use in order to decode the data blob.
*/
class Message {
    public:
        // no default constructor
        // a compile error would occur if it was used
        Message() = delete;
        
        // copy constructor
        Message(const Message& other);
        
        // copy assignment operator
        Message& operator=(const Message& other);

        // Standard constructor:
        // This allocates a buffer of the requested size and copies the data
        // referenced by the given pointer into this buffer.
        // If a size 0 is given, the pointer remains NULL.
        Message(
            std::string sender_module,
            uint16_t    msg_type,
            uint16_t    msg_size,
            void*       msg_data);

        // Constructor from buffer:
        // This is used to re-create a message from the compact binary format
        // which is used for transmission over low-bandwidth channels (e.g. modem)
        // All information is contained in the buffer (sender id, message type, length).
        Message(char* buffer);
        
        // named Constructor for a MSG_TYPE_TEXT message
        static Message TextMessage(
            std::string sender_module,
            std::string text);
            
        // Constructor for a MSG_TYPE_SYSTEM message
        static Message SystemMessage(
            std::string sender_module,
            uint32_t    time,
            uint8_t     severity_level,
            std::string text);
                    
        // Constructor for a MSG_TYPE_TELEMETRY message
        // this also creates the hash for the defined message
        // the sender must store this hash to subsequently send data messages
        static Message TelemetryMessage(
            std::string sender_module,
            uint32_t    time,
            std::string variable,
            std::string value);
                 
        // we need a destructor to free any allocated memory
        ~Message();
        
        // type reporting function
        uint16_t type() { return m_type; };
        
        // type reporting function
        uint16_t size() { return m_size; };
        
        // data extraction fuction - get a pointer to the data struct
        void* get_data() { return m_data; }; 
        
        // Generate a string with a standardized format holding the content of the message.
        std::string print_content();

        // Generate a string with a standardized format holding the message.
        // This gives the sender ID with 8 characters, separator and
        // the message content as formatted by print_content()
        // There is no CR/LF at the end of the string, a print routine has to add that if necessary.
        std::string printout();
        
        // Generate a text message with all information but the sender id serialized
        // using the printout() generated format
        Message as_text();

        // put a compact date block describing the message, suitable for transmission
        // over low-bandwidth communication channels into a given data buffer
        // it returns the number of bytes actually used
        uint8_t buffer(char* buffer, size_t size);
        
    protected:
        // there is one single member that is required for all messages
        // the sender module of the message
        std::string m_sender_module;
        uint16_t    m_type;
        uint16_t    m_size;
        void*       m_data;
};

