#include <cstdio>

#include "global.h"
#include "logger.h"
#include "message.h"

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
        Message_System msg = system_in.fetch();
        // write out
        out.transmit(msg.as_text());
        flag_system_pending = (system_in.count()>0);
    }
    
    while (flag_text_pending)
    {
        Message_Text msg = text_in.fetch();
        out.transmit(msg);
        flag_text_pending = (text_in.count()>0);
    }
    
}

TimedLogger::TimedLogger(std::string name, float rate)
{
    // copy the name
    id = name;
    // save the startup time and rate
    last_update = FC_systick_millis_count;
    log_rate = rate;
}

bool TimedLogger::have_work()
{
    float elapsed = FC_systick_millis_count - last_update;
    flag_update_pending = (elapsed*log_rate >= 1000.0);
    return flag_update_pending;
}

void TimedLogger::run()
{
    // Serial.println("TimedLogger run.");
    if (flag_update_pending)
    {
        // TODO: query the data
        Message_GPS_position msg = server_callback();
        // get the system time
        uint32_t time = FC_systick_millis_count;
        // assemble the output message
        // print the time in seconds
        char buffer[16];
        int n = snprintf(buffer, 15, "%10.3f", 0.001*time);
        buffer[n] = '\0';
        std::string text = std::string(buffer,n);
        // separator
        text += std::string(" : ");
        // append the serialized message text
        text += msg.serialize();
        // write out
        out.transmit(
            Message_Text(server_name, text)
        );
    };
    last_update = FC_systick_millis_count;
    flag_update_pending = false;
}

void TimedLogger::register_server_callback(std::function<Message_GPS_position(void)> f, std::string name)
{
    server_name = name;
    server_callback = f;
}

