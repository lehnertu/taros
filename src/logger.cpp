#include <cstdio>

#include "global.h"
#include "logger.h"
#include "message.h"

Logger::Logger(std::string name) : Module(name)
{
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    // open the connection
    flag_message_pending = false;
}

bool Logger::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (in.count()>0) flag_message_pending = true;
    return flag_message_pending;
}

void Logger::run()
{

    while (flag_message_pending)
    {
        Message msg = in.fetch();
        // write out
        text_out.transmit(msg.as_text());
        flag_message_pending = (in.count()>0);
        // system messages are also sent via the system_out port
        if (msg.type()==MSG_TYPE_SYSTEM)
        {
            system_out.transmit(msg);
        };
    }
}

Requester::Requester(std::string name, float rate) : Module(name)
{
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    // save the startup time and rate
    last_update = FC_time_now();
    log_rate = rate;
}

bool Requester::have_work()
{
    float elapsed = FC_elapsed_millis(last_update);
    flag_update_pending = (elapsed*log_rate >= 1000.0);
    return flag_update_pending;
}

void Requester::run()
{
    if (flag_update_pending)
    {
        // query the data
        Message msg = server_callback();
        // get the system time
        uint32_t time = FC_time_now();
        // assemble the output message
        // print the time in seconds
        char buffer[16];
        int n = snprintf(buffer, 15, "%10.3f", 0.001*time);
        buffer[n] = '\0';
        std::string text = std::string(buffer,n);
        // separator
        text += std::string(" : ");
        // append the serialized message text
        text += msg.print_content();
        // write out
        out.transmit(
            Message::TextMessage(server_name, text)
        );
    };
    last_update = FC_time_now();
    flag_update_pending = false;
}

void Requester::register_server_callback(std::function<Message(void)> f, std::string name)
{
    server_name = name;
    server_callback = f;
}

