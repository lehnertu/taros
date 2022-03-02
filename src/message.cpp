#include "message.h"

#include <algorithm>
#include <cstdio>


// standard constructor
Message::Message(
    std::string sender_module,
    uint8_t msg_type,
    uint16_t data_size,
    unsigned char *data
    )
{
    this->sender_module = sender_module;
    this->msg_type = msg_type;
    this->data_size = data_size;
    if (data_size>0)
    {
        // allocate memory for the data block
        this->data = new unsigned char[data_size];
        // copy the data
        std::copy(data, data+data_size, this->data);
    } else {
        this->data = NULL;
    }
}

// copy constructor
Message::Message(const Message& msg)
{
    this->sender_module = msg.sender_module;
    this->msg_type = msg.msg_type;
    this->data_size = msg.data_size;
    if (msg.data_size>0)
    {
        // allocate memory for the data block
        this->data = new unsigned char[msg.data_size];
        // copy the data
        std::copy(msg.data, msg.data + msg.data_size, this->data);
    } else {
        this->data = NULL;
    }
}

// destructor
Message::~Message()
{
    delete[] data;
}

// generate a possibly compact array of bytes for transmission over
// bandwidth-limited communication channels
std::string Message::byte_string()
{
}

// Generate a string with a standardized format holding the message.
// There is no CR/LF at the end of the string, a print routine has to add that if necessary.
std::string Message::print()
{
    std::string text = sender_module;
    // pad with spaces to 8 characters
    size_t len = text.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        text += space;
    };
    text = text.substr(0, 8);
    // separator
    text += std::string(" : ");
    // generate a text message
    Message msg = as_text();
    // append the text
    MSG_TEXT *t = (MSG_TEXT*)msg.data;
    text += t->text;
    return text;
}

// Generate a text message with all information but the sender id serialized
Message Message::as_text()
{
    // create an empty text message data block
    uint16_t size = 0;
    MSG_TEXT t { .text=std::string() };
    // fill it with the message content
    switch (msg_type)
    {
    
        case MSG_TYPE_TEXT :
        {
            t.text += ((MSG_TEXT*)data)->text;
            break;
        }
        
        case MSG_TYPE_SYSTEM :
        {
            MSG_SYSTEM *ms = (MSG_SYSTEM*)data;
            char buffer[12];
            // time
            int n = snprintf(buffer, 11, "%10.3f", (double)ms->time*0.001);
            buffer[n] = '\0';
            t.text += std::string(buffer,n);
            // separator
            t.text += std::string(" : ");
            // severity level
            n = snprintf(buffer, 5, "%4d", ms->severity_level);
            buffer[n] = '\0';
            t.text += std::string(buffer,n);
            // separator
            t.text += std::string(" : ");
            // message text
            t.text += ms->text;
            break;
        }
        
        case MSG_TYPE_TELEMETRY :
        {
            MSG_TELEMETRY *ms = (MSG_TELEMETRY*)data;
            char buffer[12];
            // time
            int n = snprintf(buffer, 11, "%10.3f", (double)ms->time*0.001);
            buffer[n] = '\0';
            t.text += std::string(buffer,n);
            // separator
            t.text += std::string(" : ");
            // vaiable name
            t.text += ms->variable.substr(0, 8);
            // separator
            t.text += std::string("=");
            // value
            t.text += ms->value;
            break;
        }
        
        case MSG_TYPE_GPS_POSITION :
        {
            MSG_GPS_POSITION *ms = (MSG_GPS_POSITION*)data;
            char buffer[16];
            // latitude
            int n = snprintf(buffer, 15, "%10.6f", ms->latitude);
            buffer[n] = '\0';
            t.text += "lat=";
            t.text += std::string(buffer,n);
            // longitude
            n = snprintf(buffer, 15, "%11.6f", ms->longitude);
            buffer[n] = '\0';
            t.text += ", long=";
            t.text += std::string(buffer,n);
            // altitude
            n = snprintf(buffer, 15, "%7.2f", ms->altitude);
            buffer[n] = '\0';
            t.text += ", alti=";
            t.text += std::string(buffer,n);
            break;
        }
        
        case MSG_TYPE_IMU_GYRO :
        {
            break;
        }
        
        case MSG_TYPE_IMU_ACCEL :
        {
            break;
        }
        
        case MSG_TYPE_IMU_MAG :
        {
            break;
        }
        
        case MSG_TYPE_IMU_AHRS :
        {
            break;
        }
        
    };
    // assemble the return message
    Message msg(
        sender_module,
        MSG_TYPE_TEXT,
        size,
        (unsigned char *) &t );
    return msg;
}


