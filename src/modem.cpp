#include "modem.h"

Modem::Modem(
    std::string name,
    uint32_t baud_rate )
{
    // copy the name
    id = name;
    // open the connection
    // Serial.begin(baud_rate);
    // Serial.println("USB_Serial setup done.");
    flag_msg_pending = false;
    flag_telemetry_pending = false;
    // send a message to the system_log
    system_log->system_in.receive(
        Message_System(id, FC_systick_millis_count, MSG_LEVEL_STATE_CHANGE, "setup done.") );
}

bool Modem::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (msg_in.count()>0) flag_msg_pending = true;
    if (telemetry_in.count()>0) flag_telemetry_pending = true;
    return flag_msg_pending | flag_telemetry_pending;
}

void Modem::run()
{

    // TODO probably we should check the buffer availability

    while (flag_msg_pending)
    {
        Message msg = msg_in.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        // Serial.write(buffer.c_str(), buffer.size());
        flag_msg_pending = (msg_in.count()>0);
    }
    
    while (flag_telemetry_pending)
    {
        Message_Telemetry msg = telemetry_in.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        // Serial.write(buffer.c_str(), buffer.size());
        flag_telemetry_pending = (telemetry_in.count()>0);
    }
}

