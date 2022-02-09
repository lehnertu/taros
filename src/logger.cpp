#include <cstdio>

#include "logger.h"

Logger::Logger(std::string name)
{
    // copy the name
    id = name;
    // open the connection
    flag_text_pending = false;
    flag_system_pending = false;
}

bool Logger::have_work()
{
    // Serial.print("Logger : ");
    // Serial.print(text_in.count());
    // Serial.print(" : ");
    // Serial.print(system_in.count());
    // Serial.println();
    // if there is something received in one of the input ports
    // we have to handle it
    if (text_in.count()>0) flag_text_pending = true;
    if (system_in.count()>0) flag_system_pending = true;
    return flag_text_pending | flag_system_pending;
}

void Logger::run()
{

    // Serial.println("Logger run.");
    while (flag_system_pending)
    {
        MESSAGE_SYSTEM msg = system_in.fetch();
        uint32_t time = system_in.fetch_time();
        std::string buffer = serialize_message(msg,time);
        // write out
        out.transmit(
            MESSAGE_TEXT { .sender_module = msg.sender_module, .text=buffer }
        );
        flag_system_pending = (system_in.count()>0);
    }
    
    while (flag_text_pending)
    {
        MESSAGE_TEXT msg = text_in.fetch();
        uint32_t time = text_in.fetch_time();
        std::string buffer = serialize_message(msg,time);
        // write out
        out.transmit(
            MESSAGE_TEXT { .sender_module = msg.sender_module, .text=buffer }
        );
        flag_text_pending = (text_in.count()>0);
    }
    
}

std::string Logger::serialize_message(MESSAGE_TEXT msg, uint32_t time)
{
    // print the time in seconds
    char buffer[16];
    int n = snprintf(buffer, 15, "%12.3f", 0.001*time);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // message text
    text += msg.text;
    return text;
}

std::string Logger::serialize_message(MESSAGE_SYSTEM msg, uint32_t time)
{
    // print the time in seconds
    char buffer[16];
    int n = snprintf(buffer, 15, "%12.3f", 0.001*time);
    buffer[n] = '\0';
    std::string text = std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // print the severity level
    n = snprintf(buffer, 15, "%4d", msg.severity_level);
    buffer[n] = '\0';
    text += std::string(buffer,n);
    // separator
    text += std::string(" : ");
    // message text
    text += msg.text;
    return text;
}


