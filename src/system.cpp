#include "system.h"
#include <Arduino.h>
#include "dummy_gps.h"
#include "serial_usb.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    USB_Serial *usb = new USB_Serial("USB_1", 9600);
    module_list->push_back(usb);
    // send a message to it
    usb->text_in.receive(
        MESSAGE_TEXT { .sender_module = "SYSTEM", .text="running setup." } );

    // Serial.println("FC_build_system");
    DummyGPS *gps = new DummyGPS("DGPS_1", 2.0);
    gps->status_out->set_receiver(&(usb->text_in));
    module_list->push_back(gps);
    
    // All messages are still just queued in the USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    usb->text_in.receive(
        MESSAGE_TEXT { .sender_module = "SYSTEM", .text="setup complete." } );
    
}

