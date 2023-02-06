#include "serial_usb.h"
#include "base.h"
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
    // send a message to the system_log
    system_log->in.receive(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "up and running.") );
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

void USB_Serial::interrupt()
{
    if (runlevel_ == MODULE_RUNLEVEL_OPERATIONAL)
        if (in.count()>0)
            schedule_task(this, std::bind(&USB_Serial::handle_MSG, this));
}

void USB_Serial::handle_MSG()
{
    // TODO probably we should check the buffer availability
    Message msg = in.fetch();
    std::string buffer = msg.printout();
    buffer += std::string("\r\n");
    // write out
    // the write is buffered and should return immediately
    Serial.write(buffer.c_str(), buffer.size());
}

