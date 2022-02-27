#include "global.h"
#include "system.h"
#include "dummy_gps.h"
#include "console_out.h"
#include "file_writer.h"

// for debugging
#include <iostream>

void FC_build_system(
    std::list<Module*> *module_list
)
{
    // create the USB serial output channel
    // USB_Serial *usb = new USB_Serial(std::string("USB_1"), 9600);
    Console_out *cout = new Console_out(std::string("COUT"));
    module_list->push_back(cout);
    // wire the syslog output to it
    system_log->out.set_receiver(&(cout->text_in));

    // create a writer for a log file
    FileWriter *log_writer = new FileWriter("LOGFILE", "taros.system.");
    module_list->push_back(log_writer);
    // wire the syslog output to it
    system_log->out.set_receiver(&(log_writer->text_in));
    
    // create a simulated GPS module
    DummyGPS *gps = new DummyGPS(std::string("GPS_1"), 5.0, 1.0);
    gps->status_out.set_receiver(&(system_log->system_in));
    gps->tm_out.set_receiver(&(cout->telemetry_in));
    module_list->push_back(gps);
    
    // create a logger capturing telemetry data at specified rate
    TimedLogger *tlog = new TimedLogger(std::string("LOG_5S"), 0.2);
    tlog->out.set_receiver(&(cout->text_in));
    tlog->out.set_receiver(&(log_writer->text_in));
    auto callback = std::bind(&DummyGPS::get_position, gps); 
    tlog->register_server_callback(callback,"GPS_1");
    module_list->push_back(tlog);

    // All start-up messages are still just queued in the Logger and USB_serial module.
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
        std::cout << "deleting " << mod->id << std::endl;
        delete mod;
    };
}
