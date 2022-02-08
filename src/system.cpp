#include "global.h"
#include "system.h"
#include "dummy_gps.h"
#include "console_out.h"
#include "file_writer.h"

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    // USB_Serial *usb = new USB_Serial(std::string("USB_1"), 9600);
    Console_out *cout = new Console_out(std::string("COUT"));
    module_list->push_back(cout);
    // wire the syslog output to it
    system_log.out.set_receiver(&(cout->text_in));

    // create a writer for a log file
    FileWriter *log_writer = new FileWriter("LOGFILE", "taros.system.");
    module_list->push_back(log_writer);
    // wire the syslog output to it
    system_log.out.set_receiver(&(log_writer->text_in));
    
    // create a simulated GPS module
    DummyGPS *gps = new DummyGPS(std::string("GPS_1"), 5.0, 1.0);
    gps->status_out.set_receiver(&(cout->text_in));
    gps->tm_out.set_receiver(&(cout->telemetry_in));
    module_list->push_back(gps);
    
    // All start-up messages are still just queued in the Logger and USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log.system_in.receive(
        MESSAGE_SYSTEM {
            .sender_module = "SYSTEM",
            .severity_level = MSG_LEVEL_MILESTONE,
            .text="build complete." } );
}

