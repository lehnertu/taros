#include "global.h"
#include "system.h"
#include "dummy_gps.h"
#include "serial_usb.h"
#include "blink.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    USB_Serial *usb = new USB_Serial(std::string("USB_1"), 9600);
    module_list->push_back(usb);
    // wire the syslog output to it
    system_log.out.set_receiver(&(usb->text_in));

    // create a module for LED blinking
    Blink *bl = new Blink(std::string("LED1"), 3.0);
    module_list->push_back(bl);
    
    // create a simulated GPS module
    DummyGPS *gps = new DummyGPS(std::string("GPS_1"), 5.0, 2.0);
    gps->status_out.set_receiver(&(usb->text_in));
    gps->tm_out.set_receiver(&(usb->telemetry_in));
    module_list->push_back(gps);
    
    // All messages are still just queued in the Logger and USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log.system_in.receive(
        MESSAGE_SYSTEM {
            .sender_module = "SYSTEM",
            .severity_level = MSG_LEVEL_MILESTONE,
            .text="setup complete." } );
}

