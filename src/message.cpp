#include "message.h"
#include <cstdio>

template<>
std::string serialize_message(MESSAGE_TEXT msg)
{
    std::string buffer = msg.sender_module;
    // pad with spaces to 8 characters
    size_t len = buffer.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        buffer += space;
    };
    buffer = buffer.substr(0, 8);
    // separator
    buffer += std::string(" : ");
    // message text
    buffer += msg.text;
    return buffer;
}

template<>
std::string serialize_message(MESSAGE_TELEMETRY msg)
{
    // sender module
    std::string buffer = msg.sender_module;
    // pad with spaces to 8 characters
    size_t len = buffer.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        buffer += space;
    };
    buffer = buffer.substr(0, 8);
    // separator
    buffer += std::string(" : ");
    // vaiable name
    std::string var = msg.variable;
    // pad with spaces to 8 characters
    len = var.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        var += space;
    };
    buffer += var.substr(0, 8);
    // separator
    buffer += std::string(" : ");
    // value
    buffer += msg.value;
    return buffer;
}

template<>
std::string serialize_message(MESSAGE_GPS_POSITION msg)
{
    char buffer[16];
    // latitude
    int n = snprintf(buffer, 15, "%10.6f", msg.latitude);
    buffer[n] = '\0';
    std::string text = "lat=";
    text += std::string(buffer,n);
    // longitude
    n = snprintf(buffer, 15, "%11.6f", msg.longitude);
    buffer[n] = '\0';
    text += ", lon=";
    text += std::string(buffer,n);
    // altitude
    n = snprintf(buffer, 15, "%7.2f", msg.altitude);
    buffer[n] = '\0';
    text += ", alt=";
    text += std::string(buffer,n);
    return text;
}

