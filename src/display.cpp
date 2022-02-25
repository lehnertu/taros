#include <Adafruit_GFX.h>
#include <SPI.h>

#include "global.h"
#include "display.h"

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// Display states
#define DISPLAY_UNINITIALIZED   0
#define DISPLAY_INITIALIZED     1
#define DISPLAY_CLOCK           2
#define DISPLAY_TTC             3
#define DISPLAY_UPDATED         4
#define DISPLAY_CLEARED         5

DisplaySSD1331::DisplaySSD1331(
    std::string name,       // the ID of the module
    float rate              // the update rate of the display
    )
{
    // copy the name
    id = name;
    update_rate = rate;
    // create the display
    display = new Adafruit_SSD1331(&SPI, DISPLAY_CS, DISPLAY_DC, DISPLAY_RST);
    // initialize the state
    last_update = FC_systick_millis_count;
    state = DISPLAY_UNINITIALIZED;
}

bool DisplaySSD1331::have_work()
{
    bool ret = false;
    switch(state)
    {
        // not yet initialized, we have to do that
        case DISPLAY_UNINITIALIZED :
        {
            ret = true;
            break;
        }
        // wait a second after initialization
        case DISPLAY_INITIALIZED :
        {
            ret = (FC_systick_millis_count-last_update>1000);
            break;
        }
        // as long as we are in the CLOCK state we always have a character to print
        case DISPLAY_CLOCK :
        {
            ret = true;
            break;
        }
        // as long as we are in the TTC state we always have a character to print
        case DISPLAY_TTC :
        {
            ret = true;
            break;
        }
        // display is updated, wait according to rate
        case DISPLAY_UPDATED :
        {
            ret = ((FC_systick_millis_count-last_update)*update_rate>1000);
            break;
        }
        // display has been cleared, need to rewrite
        case DISPLAY_CLEARED :
        {
            state = DISPLAY_CLOCK;
            cycle_count = 0;
            ret = true;
            break;
        }
    };
    return ret;
}

void DisplaySSD1331::run()
{
    switch(state)
    {
        // not yet initialized, we have to do that
        case DISPLAY_UNINITIALIZED :
        {
            display->begin();
            // send a start-up message
            last_update = FC_systick_millis_count;
            state=DISPLAY_INITIALIZED;
            break;
        }
        // a second after initialization we clear the display
        // TODO introduce the wait
        // TODO clearing the display this way leads to a systick overrun
        case DISPLAY_INITIALIZED :
        {
            display->fillScreen(BLACK);
            last_update = FC_systick_millis_count;
            // TODO: sending via status_out leads to a fault
            // status_out.transmit(
            //     Message_System(id, MSG_LEVEL_STATE_CHANGE, std::string("initialized.")) );
            system_log.system_in.receive(
                Message_System(id, MSG_LEVEL_STATE_CHANGE, "initialized.") );
            state=DISPLAY_CLEARED;
            break;
        }
        // handle the clock display
        case DISPLAY_CLOCK :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 3);
                // in the first cycle generate the string
                uint32_t time = FC_systick_millis_count;
                uint8_t h = time/(1000*60*60);
                time -= h*60*60*1000;
                uint8_t m = time/(1000*60);
                time -= m*60*1000;
                float s = time/1000.0; 
                num_cycles = snprintf(buffer, 16, "%02d:%02d:%06.3f", h,m,s);
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with the clock
                cycle_count = 0;
                state=DISPLAY_TTC;
            }
            break;
        }
        // handle the ttc display
        case DISPLAY_TTC :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 11);
                // in the first cycle generate the string
                float ttc = 1.0e6 * (float)FC_max_time_to_completion / (float)F_CPU_ACTUAL;
                FC_max_time_to_completion = 0;
                num_cycles = snprintf(buffer, 16, "ttc : %6.1f us", ttc);
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with the ttc
                cycle_count = 0;
                state=DISPLAY_UPDATED;
                last_update = FC_systick_millis_count;
                state=DISPLAY_UPDATED;
            }
            break;
        }
        // display has been updated - wait
        // action occurs when the update has expired, clear the display
        // TODO clearing the display this way leads to a systick overrun
        case DISPLAY_UPDATED :
        {
            // a) display->fillScreen(BLACK);
            // b) display->fillRect(0, 0, display->TFTWIDTH, display->TFTHEIGHT, BLACK);
            // c) use SSD1331 command #22h
            display->sendCommand(SSD1331_CMD_FILL);
            display->sendCommand(1);
            display->sendCommand(SSD1331_CMD_DRAWRECT);
            display->sendCommand(0);    // first column
            display->sendCommand(0);    // first row
            display->sendCommand(95);   // last column
            display->sendCommand(22);   // last row
            display->sendCommand(63);   // outline R
            display->sendCommand(0);    // outline G
            display->sendCommand(0);    // outline B
            display->sendCommand(0);    // fill R
            display->sendCommand(0);    // fill G
            display->sendCommand(10);   // fill B
            last_update = FC_systick_millis_count;
            state=DISPLAY_CLEARED;
            break;
        }
    };
}

