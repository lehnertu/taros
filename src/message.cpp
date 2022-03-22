#include "message.h"
#include <cstdio>

Message::Message(std::string sender_module)
{
    m_sender_module = sender_module;
}

std::string print_content()
{
    return std::string("");
}

std::string Message::printout()
{
    std::string text = m_sender_module;
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
    // message text
    text += print_content();
    return text;
}

Message_Text Message::as_text()
{
    return Message_Text(m_sender_module, print_content());
}


//-----------------------------------------------------------------------------


Message_Text::Message_Text(
    std::string sender_module,
    std::string text) :
    // call the base class contructor
    Message(sender_module)
{
    m_text = text;
}

std::string Message_Text::print_content()
{
    return m_text;
}

//-----------------------------------------------------------------------------

Message_System::Message_System(
    std::string sender_module,
    uint32_t time,
    uint8_t severity_level,
    std::string text) :
    // call the base class contructor
    Message(sender_module)
{
    m_severity_level = severity_level;
    m_time = time;
    m_text = text;
}

std::string Message_System::print_content()
{
    char buffer[12];
    // time
    int n = snprintf(buffer, 11, "%10.3f", (double)m_time*0.001);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // severity level
    n = snprintf(buffer, 5, "%4d", m_severity_level);
    buffer[n] = '\0';
    text += std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // message text
    text += m_text;
    return text;
}

//-----------------------------------------------------------------------------

Message_GPS_position::Message_GPS_position(
    std::string sender_id,
    double  latitude,
    double  longitude,
    float   altitude) :
    // call the base class contructor
    Message(sender_id)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_altitude = altitude;
}

std::string Message_GPS_position::print_content()
{
    char buffer[16];
    // latitude
    int n = snprintf(buffer, 15, "%10.6f", m_latitude);
    buffer[n] = '\0';
    std::string text = "lat=";
    text += std::string(buffer,n);
    // longitude
    n = snprintf(buffer, 15, "%11.6f", m_longitude);
    buffer[n] = '\0';
    text += ", long=";
    text += std::string(buffer,n);
    // altitude
    n = snprintf(buffer, 15, "%7.2f", m_altitude);
    buffer[n] = '\0';
    text += ", alti=";
    text += std::string(buffer,n);
    return text;
}

//-----------------------------------------------------------------------------

Message_Telemetry::Message_Telemetry(
            std::string sender_id,
            uint32_t time,
            std::string variable,
            std::string value) :
    // call the base class contructor
    Message(sender_id)
{
    m_time = time;
    m_variable = variable;
    m_value = value;
}

std::string Message_Telemetry::print_content()
{
    char buffer[12];
    // time
    int n = snprintf(buffer, 11, "%10.3f", (double)m_time*0.001);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // vaiable name
    std::string varname = m_variable;
    // pad with spaces to 8 characters
    size_t len = varname.size();
    if (len<8)
    {
        std::string space(8-len, ' ');
        varname += space;
    };
    text += varname.substr(0, 8);
    // separator
    text += std::string(" : ");
    // value
    text += m_value;
    return text;
}


