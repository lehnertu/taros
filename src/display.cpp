#include <Adafruit_GFX.h>
#include <SPI.h>

#include "base.h"
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
#define DISPLAY_CLEAR           2
#define DISPLAY_CLOCK           3
#define DISPLAY_ITC             4
#define DISPLAY_TTC             5
#define DISPLAY_HEADING         6
#define DISPLAY_PITCH           7
#define DISPLAY_COMPLETE       10

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
    // set some defaults
    heading = 123.0;
    pitch = 90.1;
    roll = 14.5;
    gx = -49.1;
    gy = -5.3;
    gz = 13.2;

    // initialize the state
    last_update = FC_time_now();
    state = DISPLAY_UNINITIALIZED;
    flag_update_running = false;
}

void DisplaySSD1331::setup()
{
    // this seems to use the default transfer rate of 8 MHz
    display->begin();
    last_update = FC_time_now();
    // wait one second
    while (FC_elapsed_millis(last_update)<1000) {};
    display->fillScreen(BLACK);
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
    state=DISPLAY_COMPLETE;
    
    flag_update_running = false;
    
    last_update = FC_time_now();
    runlevel_ = MODULE_RUNLEVEL_OPERATIONAL;
};

void DisplaySSD1331::interrupt()
{
    if (runlevel_ == MODULE_RUNLEVEL_OPERATIONAL)
    {
        // If there is something received in one of the input ports we have to handle it.
        // We can do that right here as it takes almost no time
        if (data_in.count()>0)
        {
            Message msg = data_in.fetch();
            // process the messages we can handle
            if (msg.type()==MSG_TYPE_IMU_AHRS)
            {
                DATA_IMU_AHRS *data = (DATA_IMU_AHRS*) msg.get_data();
                heading = data->heading;
                pitch = data->attitude;
                roll = data->roll;
            };
            if (msg.type()==MSG_TYPE_IMU_GYRO)
            {
                DATA_IMU_GYRO *data = (DATA_IMU_GYRO*) msg.get_data();
                gx = data->roll;
                gy = data->nick;
                gz = data->yaw;
            };
        }
        if (ahrs_in.count()>0)
        {
            DATA_IMU_AHRS data = ahrs_in.fetch();
            heading = data.heading;
            pitch = data.attitude;
            roll = data.roll;
        }
        if (gyro_in.count()>0)
        {
            DATA_IMU_GYRO data = gyro_in.fetch();
            gx = data.roll;
            gy = data.nick;
            gz = data.yaw;
        }
        // If there is an update running we have to request a task for that
        if (flag_update_running)
        {
            // insert the redraw() routine into the tasklist
            schedule_task(this, std::bind(&DisplaySSD1331::redraw, this));
        } else {
            // when due, start a new update
            if (FC_elapsed_millis(last_update)*update_rate>1000)
            {
                last_update = FC_time_now();
                state = DISPLAY_CLEAR;
                cycle_count = 0;
                // this will become effective with the next interrupt
                flag_update_running = true;
            }
        }
    }
}

// redraw gets called many times until the full display refresh has been accomplished
void DisplaySSD1331::redraw()
{
    switch(state)
    {
        // the time since the last update has expired, clear the display
        case DISPLAY_CLEAR :
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
            display->sendCommand(46);   // last row
            display->sendCommand(63);   // outline R
            display->sendCommand(0);    // outline G
            display->sendCommand(0);    // outline B
            display->sendCommand(0);    // fill R
            display->sendCommand(0);    // fill G
            display->sendCommand(10);   // fill B
            state=DISPLAY_CLOCK;
            cycle_count = 0;
            break;
        }
        // handle the clock display
        case DISPLAY_CLOCK :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 3);
                // in the first cycle generate the string
                uint32_t time = FC_time_now();
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
                state=DISPLAY_ITC;
            }
            break;
        }
        // handle the interrupt timing display
        case DISPLAY_ITC :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 11);
                // in the first cycle generate the string
                float ttc = 1.0e6 * (float)FC_max_isr_time_to_completion / (float)F_CPU_ACTUAL;
                num_cycles = snprintf(buffer, 16, "%8s %4.1fus", FC_max_isr_time_module.c_str(), ttc);
                // clear the stored value once it is diaplayed
                // TODO: if exceedingly large generate a warning message
                FC_max_isr_time_to_completion = 0;
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with the ITC
                cycle_count = 0;
                state=DISPLAY_TTC;
            }
            break;
        }
        // handle the task timing display
        case DISPLAY_TTC :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 19);
                // in the first cycle generate the string
                float ttc = 0.001*(float)FC_max_task_time_to_completion;
                num_cycles = snprintf(buffer, 16, "%8s %4.1fms", FC_max_task_time_module.c_str(), ttc);
                // clear the stored value once it is diaplayed
                // TODO: if exceedingly large generate a warning message
                FC_max_task_time_to_completion = 0;
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with the TTC
                cycle_count = 0;
                state=DISPLAY_HEADING;
            }
            break;
        }
        // handle the HEADING display
        case DISPLAY_HEADING :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 30);
                // in the first cycle generate the string
                num_cycles = snprintf(buffer, 16, "H: %5.1f  %5.1f", heading, gz);
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with the heading
                cycle_count = 0;
                state=DISPLAY_PITCH;
            }
            break;
        }
        // handle the PITCH display
        case DISPLAY_PITCH :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 38);
                // in the first cycle generate the string
                num_cycles = snprintf(buffer, 16, "P: %5.1f  %5.1f", pitch, gy);
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with pitch
                cycle_count = 0;
                state=DISPLAY_COMPLETE;
                // we are done completely, update is finished
                flag_update_running = false;
                last_update = FC_time_now();
            }
            break;
        }
        // handle the ROLL display
        /*
        case DISPLAY_ROLL :
        {
            if (cycle_count == 0)
            {
                display->setCursor(3, 22);
                // in the first cycle generate the string
                num_cycles = snprintf(buffer, 16, "R: %5.1f  %5.1f", roll, gx);
            } else {
                // in all subsequent cycles each display one character
                display->print(buffer[cycle_count-1]);
            }
            cycle_count++;
            if (cycle_count > num_cycles)
            {
                // we are done with roll
                cycle_count = 0;
                state=DISPLAY_COMPLETE;
                // we are done completely, update is finished
                flag_update_running = false;
                last_update = FC_time_now();
            }
            break;
        }
        */
    };
}

