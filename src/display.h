#pragma once

#include <cstdint>

#include "global.h"
#include <Adafruit_SSD1331.h>

// fixed pin numbers for the display according to hardware (assume there's only one)
#define DISPLAY_SCLK 13
#define DISPLAY_MOSI 11
#define DISPLAY_CS   10
#define DISPLAY_RST   6
#define DISPLAY_DC    9

/*
    Display a clock and status information on the OLED display.
    ttc is the maximum time for every systick until the task list is emptied.
    No systick overruns occur anymore (except for startup ?!).
    For clearing the display (window) a hardware-accelerated command is used.
    All strings are generated at once but transfered character by character
    to the display, one per systick.
    Thus, a complete display update takes about 40 ms (systicks).
*/
class DisplaySSD1331 : public Module
{

public:

    // constructor
    DisplaySSD1331(
        std::string name,       // the ID of the module
        float rate              // the update rate of the display
        );

    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when something needs to be drawn.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It manages all drawing.
    virtual void run();
        
    // port over which status messages are sent
    SenderPort<Message_System> status_out;

private:

    Adafruit_SSD1331 *display;
    
    float       update_rate;    // the update rate of the display
    uint8_t     state;          // state machine controlling the display update
    uint32_t    last_update;    // the time of tha last update
    int         cycle_count;    // number of cycles performed on a display print action
    int         num_cycles;     // the number of cycles (characters) to complete the print
    char        buffer[18];     // the print buffer for one display line (max. 16 characters)

};

