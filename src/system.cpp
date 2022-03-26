#include "global.h"
#include "system.h"
#include "dummy_gps.h"
#include "serial_usb.h"
#include "display.h"
#include "modem.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    USB_Serial *usb = new USB_Serial(std::string("USB_1"), 9600);
    module_list->push_back(usb);
    // wire the syslog output to it
    system_log->out.set_receiver(&(usb->text_in));

    // create a module for LED blinking
    // Blink *bl = new Blink(std::string("LED1"), 5.0);
    // module_list->push_back(bl);
    
    // create a display with 2Hz update
    DisplaySSD1331 *display = new DisplaySSD1331(std::string("DISPLAY"), 2.0);
    display->status_out.set_receiver(&(system_log->system_in));
    module_list->push_back(display);
    
    // create a simulated GPS module
    DummyGPS *gps = new DummyGPS(std::string("GPS_1"), 5.0, 2.0);
    gps->status_out.set_receiver(&(system_log->system_in));
    gps->tm_out.set_receiver(&(usb->telemetry_in));
    module_list->push_back(gps);
    
    // create a modem for comminication with a ground station
    Modem *modem = new Modem(std::string("MODEM_1"), 9600);
    modem->status_out.set_receiver(&(system_log->system_in));
    module_list->push_back(modem);
    
    // create a logger capturing telemetry data at specified rate
    TimedLogger *tlog = new TimedLogger(std::string("LOG_5S"), 0.2);
    tlog->out.set_receiver(&(usb->text_in));
    auto callback = std::bind(&DummyGPS::get_position, gps); 
    tlog->register_server_callback(callback,"GPS_1");
    module_list->push_back(tlog);

    // All messages are still just queued in the Logger and USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log->system_in.receive(
        Message_System("SYSTEM", FC_systick_millis_count, MSG_LEVEL_MILESTONE, "build complete.") );
}

void FC_destroy_system(
    std::list<Module*> *module_list
)
{
    std::list<Module*>::iterator it;
    for (it = module_list->begin(); it != module_list->end(); it++)
    {
        Module* mod = *it;
        delete mod;
    };
}
