#include "console_out.h"
#include <iostream>

Console_out::Console_out(std::string name)
{
    // copy the name
    id = name;
    flag_message_pending = false;
    // send a message to the system_log
    system_log->in.receive(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "setup done.") );
}

bool Console_out::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (in.count()>0) flag_message_pending = true;
    return flag_message_pending;
}

void Console_out::run()
{
    while (flag_message_pending)
    {
        Message msg = in.fetch();
        std::string buffer = msg.printout();
        // write out
        std::cout << buffer << std::endl;
        flag_message_pending = (in.count()>0);
    }
}

