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
#define DISPLAY_CLEARED         2
#define DISPLAY_UPDATED         3

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
            ret = true;
            break;
        // wait a second after initialization
        case DISPLAY_INITIALIZED :
            ret = (FC_systick_millis_count-last_update>1000);
            break;
        // display has been cleared, need to rewrite
        case DISPLAY_CLEARED :
            ret = true;
            break;
        // display is updated, wait according to rate
        case DISPLAY_UPDATED :
            ret = ((FC_systick_millis_count-last_update)*update_rate>1000);
            break;
    };
    return ret;
}

void DisplaySSD1331::run()
{
    switch(state)
    {
        // not yet initialized, we have to do that
        case DISPLAY_UNINITIALIZED :
            display->begin();
            // send a start-up message
            status_out.transmit(
                Message_Text(id, std::string("initialized.")) );
            last_update = FC_systick_millis_count;
            state=DISPLAY_INITIALIZED;
            break;
        // a second after initialization we clear the display
        case DISPLAY_INITIALIZED :
            display->fillScreen(BLACK);
            last_update = FC_systick_millis_count;
            state=DISPLAY_CLEARED;
            break;
        // display has been cleared, need to rewrite
        case DISPLAY_CLEARED :
            display->setCursor(3, 3);
            display->print(FC_systick_millis_count);
            display->println(" ms");
            display->print(FC_max_time_to_completion);
            FC_max_time_to_completion = 0;
            display->println(" us");
            status_out.transmit(
                Message_Text(id, std::string("updated.")) );
            last_update = FC_systick_millis_count;
            state=DISPLAY_UPDATED;
            break;
        // display update has expired, clear it
        // TODO clearing the display this way leads to a systick overrun
        case DISPLAY_UPDATED :
            // a) display->fillScreen(BLACK);
            // b) display->fillRect(0, 0, display->TFTWIDTH, display->TFTHEIGHT, BLACK);
            // c) use SSD1331 command #22h
            display->sendCommand(SSD1331_CMD_FILL);
            display->sendCommand(1);
            display->sendCommand(SSD1331_CMD_DRAWRECT);
            display->sendCommand(0);    // first column
            display->sendCommand(0);    // first row
            display->sendCommand(60);   // last column
            display->sendCommand(30);   // last row
            display->sendCommand(63);   // outline R
            display->sendCommand(0);    // outline G
            display->sendCommand(0);    // outline B
            display->sendCommand(0);    // fill R
            display->sendCommand(0);    // fill G
            display->sendCommand(10);   // fill B
            status_out.transmit(
                Message_Text(id, std::string("cleared.")) );
            last_update = FC_systick_millis_count;
            state=DISPLAY_CLEARED;
            break;
    };
}

