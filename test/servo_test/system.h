#pragma once

#include <cstdint>
#include <list>

#include "module.h"
#include "message.h"
#include "modem.h"
#include "servo.h"
#include "servo_tester.h"

// all modules that will be included during the system build
extern Servo8chDriver *servo;
extern ServoTester *tester;

// -- actually defined in main.cpp --
extern Logger* system_log;
extern FileWriter* system_log_file_writer;

/*
    This is the system definition. All modules are created and
    inserted into the list. The connections between modules are defined
    and registered along with the message type.
*/
void FC_build_system(
    std::list<Module*> *module_list
);

/*
    This cleans up the system. In a practical environment there is an
    infinite event loop, so, this will never be reached.
    For debugging, the loop can be ended and the sastem cleaned up
    to allow the detection of other sources of memory leaks.
*/
void FC_destroy_system(
    std::list<Module*> *module_list
);
