#include "message.h"

template<>
std::string serialize_message<MESSAGE_TEXT>(MESSAGE_TEXT msg)
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
std::string serialize_message<MESSAGE_TELEMETRY>(MESSAGE_TELEMETRY msg)
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

