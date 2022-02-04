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
    flag_text_pending = false;
    flag_telemetry_pending = false;
}

bool USB_Serial::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (text_in.count()>0) flag_text_pending = true;
    if (telemetry_in.count()>0) flag_telemetry_pending = true;
    return flag_text_pending | flag_telemetry_pending;
}

void USB_Serial::run()
{

    // TODO probably we should check the buffer availability
    uint32_t usec_start = micros();

    while (flag_text_pending)
    {
        MESSAGE_TEXT msg = text_in.fetch();
        std::string buffer = serialize_text_message(msg);
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        flag_text_pending = (text_in.count()>0);
    }
    
    while (flag_telemetry_pending)
    {
        MESSAGE_TELEMETRY msg = telemetry_in.fetch();
        std::string buffer = serialize_telemetry_message(msg);
        // write out
        // the write is buffered and returns immediately
        Serial.write(buffer.c_str(), buffer.size());
        flag_telemetry_pending = (telemetry_in.count()>0);
    }
    
    uint32_t usec_stop = micros();
    Serial.print("  --> transmission took ");
    Serial.print(usec_stop-usec_start);
    Serial.println(" us");
    
}

std::string USB_Serial::serialize_text_message(MESSAGE_TEXT msg)
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
    buffer += std::string("\r\n");
    return buffer;
}

std::string USB_Serial::serialize_telemetry_message(MESSAGE_TELEMETRY msg)
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
    buffer += std::string("\r\n");
    return buffer;
}

