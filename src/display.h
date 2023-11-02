#pragma once

#include <cstdint>
#include "module.h"
#include "port.h"
 
#include "stream.h"
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
    Thus, a complete display update takes about 100 ms (systicks).
*/
class DisplaySSD1331 : public Module
{

public:

    // constructor
    DisplaySSD1331(
        std::string name,       // the ID of the module
        float rate              // the update rate of the display
        );

    // TODO: move initializations here
    virtual void setup();
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when something needs to be drawn.
    virtual bool have_work() { return false; };
    virtual void interrupt();
    
    // This is the worker function being executed by the taskmanager.
    // It manages all drawing.
    virtual void redraw();
    
    virtual void run() {};
        
    // port over which status messages are sent
    SenderPort status_out;

    // port at which arbitrary messages are received
    // messages that contain valid data for display are processed
    // data are stored internally and will be updated during the next display cycle
    ReceiverPort data_in;

    // a receiver for data streams
    StreamReceiver<DATA_IMU_AHRS> ahrs_in;
    StreamReceiver<DATA_IMU_GYRO> gyro_in;
    
private:

    Adafruit_SSD1331 *display;
    
    // if an update is due or running
    bool        flag_update_running;
    
    // the data on display
    float       heading;
    float       pitch;
    float       roll;
    float       gx, gy, gz;
    
    float       update_rate;    // the update rate of the display
    uint8_t     state;          // state machine controlling the display update
    uint32_t    last_update;    // the time of the last update
    int         cycle_count;    // number of cycles performed on a display print action
    int         num_cycles;     // the number of cycles (characters) to complete the print
    char        buffer[18];     // the print buffer for one display line (max. 16 characters)

};

