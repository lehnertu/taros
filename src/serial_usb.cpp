#include "serial_usb.h"
#include <Arduino.h>

USB_Serial::USB_Serial(
    std::string name,
    uint32_t baud_rate )
{
    // copy the name
    id = name;
    // open the connection
    Serial.begin(baud_rate);
    Serial.println("\n\n  Teensy Flight Controller.");
    Serial.println("USB_Serial setup done.");
}

bool USB_Serial::have_work()
{
    // Serial.println("USB_Serial have_work()");
    // if there is something received in text_in we have to handle it
    return (text_in.count()>0);
}

void USB_Serial::run()
{
    // Serial.println("USB_Serial run()");
    MESSAGE_TEXT msg = text_in.fetch();
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
    buffer += std::string("\r\n");
    // write out
    // the write is buffered and returns immediately
    // TODO probably we should check the buffer availability
    // uint32_t usec_start = micros();
    Serial.write(buffer.c_str(), buffer.size());
    // uint32_t usec_stop = micros();
    // Serial.print("transmission took ");
    // Serial.print(usec_stop-usec_start);
    // Serial.println(" us");
}
