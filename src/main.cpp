#include <string>
#include <list>
#include <sstream>

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h

// TODO: the program seems to crash, when the serial USB connection is not present
// it reproducibly stops, when the terminal connection is broken from computer side
// maybe also when the SD card is missing - not sure

#include "kernel.h"
#include "global.h"
#include "display.h"
#include "module.h"
#include "message.h"
#include "system.h"

#ifndef VERSION_MAJOR
#define VERSION_MAJOR 0
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_BUILD
#define VERSION_BUILD 0
#endif

#ifdef USE_USB_SERIAL
# include "usb_serial.h"
// we declare an alias of the Serial port
// the actual definition is in main.cpp
usb_serial_class usb_serial_debug;
#endif

bool SD_card_OK;
int SD_file_No;
Logger *system_log;
FileWriter* system_log_file_writer = 0;

extern "C" int main(void)
{

	// use the USB serial connection to send debugging information from a possible system crash
	// during the previous run of the system
	// this is for debugging purpuse and should be disabled for production systems
	// How comes this is known, anyway ?
#ifdef USE_USB_SERIAL
	usb_serial_debug = Serial;
	usb_serial_debug.begin(115200);
	usb_serial_debug.print(CrashReport);
    usb_serial_debug.print('\n');
# endif
	
    // the logger has to be added to the list of modules so it will be scheduled for execution
    system_log = new Logger("SYSLOG");
    module_list.push_back(system_log);
    // the logger can queue messages even before its setup() is run
    std::stringstream msg;
    msg << "Teensy Flight Controller - Version ";
    msg << VERSION_MAJOR << "." << VERSION_MINOR << " - Build #" << VERSION_BUILD;
#ifdef USE_USB_SERIAL
	std::string buffer = msg.str();
    buffer += std::string("\r\n");
    usb_serial_debug.write(buffer.c_str(), buffer.size());
#endif
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, msg.str()) );
    
    // we initialize the SD card here as some modules may want to read or
    // write data during setup
    SD_card_OK = SD.begin(BUILTIN_SDCARD);
    if (SD_card_OK)
    {
        system_log->in.receive(
            Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "found SD card.") );
        // search through the files to determine the run number
        SD_file_No = 0;
        char syslog_filename[40];
        sprintf(syslog_filename, "taros.%05d.system.log", SD_file_No);
        while (SD.open(syslog_filename, FILE_READ))
        {
            // file already exists, use next number
            SD_file_No++;
            sprintf(syslog_filename, "taros.%05d.system.log", SD_file_No);
        };
        // create a file writer
        system_log_file_writer = new FileWriter("SYSLOGF",std::string(syslog_filename));
        system_log_file_writer->setup();
        // TODO: what if a problem occurs ?
        // module_list.push_back(system_log_file_writer);
        // wire the syslog output to the file
        // system_log->text_out.set_receiver(&(system_log_file_writer->in));
    }
    else
    {
        system_log->in.receive(
            Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_ERROR, "SD card not found.") );
        // TODO: in this case we have to create a dummy FileWriter
    };
    
    // Here the core hardware is initialized.
    // The millisecond systick interrupt is bent to our own ISR.
    // the interrupt shouldn't be active during the setup stage
    // for some (unclear) reason this needs to be done before the module setup.
    // We have introduced a flag that is initially false
    // indicating whether our own systick interrupt can actually call modules.
    setup_core_system();
	
	// Now we create all modules
	FC_init_system();
    // Call the setup() method for all modules.
    // All modules that are properly initialized get included in the list
    FC_setup_system(&module_list);
    // Now we wire all modules into the final system
    FC_build_system();
    // Complete system ready to go
    
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "entering event loop.") );
        
    // delayMicroseconds(100);
    
    // only now should our systick_ISR be allowed to call module interrupts
    FC_module_interrupts_activate();
    
	// infinite system loop
	kernel_loop();
	
    // we will never get here, except when limiting the task-loop for testing
    FC_destroy_system(&module_list);
    
};

