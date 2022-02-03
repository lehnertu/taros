#include "serial_usb.h"
#include <Arduino.h>

USB_Serial::USB_Serial(
    char const *name,
    uint32_t baud_rate )
{
    // copy the name
    strncpy(id,name,8);
    id[8] = 0;
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
    strncpy(buffer, msg.sender_module, 8);
    size_t count = strlen(msg.sender_module);
    char *bp = buffer + count;
    // fill with spaces
    while (count<8)
    {
        *bp++ = ' ';
        count++;
    };
    *bp++ = ':';
    *bp++ = ' ';
    // we have used 10 characters, 245 remaining plus the terminating zero
    strncpy(bp, msg.text, 245);
    // write out
    Serial.println(buffer);
}
