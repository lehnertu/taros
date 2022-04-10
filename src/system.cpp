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
    system_log->out.set_receiver(&(cout->in));

    // create a writer for a log file
    FileWriter *log_writer = new FileWriter("LOGFILE", "taros.system.");
    module_list->push_back(log_writer);
    // wire the syslog output to it
    system_log->out.set_receiver(&(log_writer->in));
    
    // create a simulated GPS module
    DummyGPS *gps = new DummyGPS(std::string("GPS_1"), 5.0, 1.0);
    gps->status_out.set_receiver(&(system_log->in));
    gps->tm_out.set_receiver(&(cout->in));
    module_list->push_back(gps);
    
    // create a logger capturing telemetry data at specified rate
    Requester *req = new Requester(std::string("LOG_5S"), 0.2);
    req->out.set_receiver(&(cout->in));
    req->out.set_receiver(&(log_writer->in));
    auto callback = std::bind(&DummyGPS::get_position, gps); 
    req->register_server_callback(callback,"GPS_1");
    module_list->push_back(req);

    // All start-up messages are still just queued in the Logger and USB_serial module.
    // They will get sent now, when the scheduler and taskmanager pick up their work.
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "build complete.")
    );

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
