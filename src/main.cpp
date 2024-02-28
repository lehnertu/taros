#include <cstdlib>
#include <cstdint>
#include <string>
#include <list>

// the main program is independent from the Arduino core system and libraries
// TODO: it comes in idirectly via display.h -> Adafruit_SSD1331.h
// TODO: we need ARM_DWT_CYCCNT

// TODO: the program seems to crash, when the serial USB connection is not present
// it reproducibly stops, when the terminal connection is broken from computer side
// maybe also when the SD card is missing - not sure

#include "base.h"
#include "global.h"
#include "display.h"
#include "module.h"
#include "message.h"
#include "system.h"

bool SD_card_OK;
int SD_file_No;
Logger *system_log;
FileWriter* system_log_file_writer = 0;

extern "C" int main(void)
{

    // the logger has to be added to the list of modules so it will be scheduled for execution
    system_log = new Logger("SYSLOG");
    module_list.push_back(system_log);
    // the logger can queue messages even before its setup() is run
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE,"Teensy Flight Controller - Version 1.0") );
    
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
    // TODO: this should be done only just before entering the event loop
    // the interrupt shouldn't be active during the setup stage
    // for some (unclear) reason this needs to be done before the module setup.
    // If that's true, we should introduce a flag that indicates whether our own
    // systick interrupt can actually call modules.
    setup_core_system();

	// Now we create all modules
	FC_init_system();
    // Call the setup() method for all modules.
    // All modules that are properly initialized get included in the list
    FC_setup_system(&module_list);
    // Now we wire all modules into the final system
    FC_build_system();
    // Complete system ready to go
    
    // now is the time to hand the flow control to the commander
    // TODO: the commander should do that himself when he gets a chance to work
    // commander.activate();
    
    system_log->in.receive(
        Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_MILESTONE, "entering event loop.") );
	// infinite system loop
	// while (FC_time_now()<12000)
	while(true)
	{
	
	    // watchdog - once per second
	    if (FC_systick_millis_count % 1000 == 837)
	    {
	        float delay = 1.0e6 * (float)FC_max_isr_spacing / (float)F_CPU_ACTUAL;
	        FC_max_isr_spacing = 0;
	        if (delay>1100.0)
	        {
	            char numStr[20];
                sprintf(numStr,"%7.1f", delay);
                std::string text("delayed systick (spacing "+std::string(numStr)+" us)!");
                system_log->in.receive(
                    Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_CRITICAL, text) );
	        }
	        
	    }
	    
	    // TASKMANAGER:
	    // All scheduled tasks get executed based on priority (list position).
	    // It should be guaranteed that any scheduled task is executed within 10 ms.
        // No task should run longer than 5ms.
        // Both constraints are not actively inforced, but any violations are reported.	    
        if (!task_list.empty())
        {
            // get the first entry in the task list
            Task task = task_list.front();
            // remove it from the list
            task_list.pop_front();
            // check how much time has elapsed from the request of the task
            uint32_t start_delay = FC_systick_millis_count-task.request_time;
            // report anything exceeding 10 ms
            if (start_delay>10000)
            {
                char numStr[20];
                sprintf(numStr,"%5.1f",0.001*(float)start_delay);
                std::string text(task.module->id+" task start delayed by  "+std::string(numStr)+" ms");
                system_log->in.receive(
                    Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_CRITICAL, text) );;
            };
            // execute the task
            uint32_t start = micros();
            task.funct();
            uint32_t stop = micros();
            // the difference automaticall wraps around
            uint32_t runtime = stop - start;
            // check the runtime of the task
            if (runtime>FC_max_task_time_to_completion)
            {
                // the max is reset when an output is created (either log or display)
                FC_max_task_time_module = task.module->id;
                FC_max_task_time_to_completion = runtime;
            };
            // if execution time exceeds 5ms the offending module is reported
            if (runtime>5000)
            {
                char numStr[20];
                sprintf(numStr,"%d",(int)(stop-start));
                std::string text(task.module->id+" task runtime "+std::string(numStr)+" us");
                system_log->in.receive(
                    Message::SystemMessage("SYSTEM", FC_time_now(), MSG_LEVEL_CRITICAL, text) );;
            };
        };

	}; // infinite system loop

    // we will never get here, except when limiting the task-loop for testing
    FC_destroy_system(&module_list);
    
};

