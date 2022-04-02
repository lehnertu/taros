#include "serial_usb.h"
#include <Arduino.h>

USB_Serial::USB_Serial(
    std::string name,
    uint32_t baud_rate )
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_INITALIZED;
    // open the connection
    Serial.begin(baud_rate);
    // Serial.println("USB_Serial setup done.");
    flag_msg_pending = false;
    // send a message to the system_log
    system_log->in.receive(
        Message(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

bool USB_Serial::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (in.count()>0) flag_msg_pending = true;
    return flag_msg_pending;
}

void USB_Serial::run()
{

    // TODO probably we should check the buffer availability

    while (flag_msg_pending)
    {
        Message msg = in.fetch();
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        flag_msg_pending = (in.count()>0);
    }
}

