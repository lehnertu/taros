#pragma once

#include <cstdint>
#include <list>

#include "module.h"
#include "message.h"
#include "dummy_gps.h"
#include "display.h"
#include "modem.h"
#include "motion.h"
#include "servo.h"

// all modules that will be included during the system build
extern DisplaySSD1331 *display;
extern StreamFileWriter* fast_log_file_writer;
extern DummyGPS *gps;
extern MotionSensor *imu;
extern Modem *modem;

// -- actually defined in main.cpp --
extern Logger* system_log;
// extern USB_Serial *usb;
extern FileWriter* system_log_file_writer;

/*
    This is the system definition. All modules are created and
    wired to the system_log. This only calls the constructors - nothing
    should go wrong here. No connections other than to system_log are made yet.
*/
void FC_init_system();

/*
	This fills the list of modules making up the system.
    For every module the setup() method is called.
    At that time messages to system_log are possible.
    Only modules that report a state of MODULE_RUNLEVEL_SETUP_OK after setup()
    will be included in the system and included in the list of modules.
*/
void FC_setup_system(
    std::list<Module*> *module_list
);

/*
    This is the system definition.
    All modules (if properly active) are wired to each other.
    This has to be done in an appropriate sequence, such that modules are
    setup only after other modules they may rely on.
*/
void FC_build_system();

/*
    This cleans up the system. In a practical environment there is an
    infinite event loop, so, this will never be reached.
    For debugging, the loop can be ended and the sastem cleaned up
    to allow the detection of other sources of memory leaks.
*/
void FC_destroy_system(
    std::list<Module*> *module_list
);
