/*  
    A module is a software object handling one particular task or subsystem
    in the FlightController system. Apart from scheduler and taskmanager
    everything is a module. Here we declare an abstract class from which
    all modules will be derived.
    
    All modules will be kept in a list and cyclically be asked by the scheduler
    if they have some task to execute. In that case the task will be inserted
    into the list of pending tasks with an appropriate priority.
    The taskmanager will handle all scheduled tasks and bring them to execution.
    
    Modules can communicate by sending messages to other modules.
    A sent message will be stored in the input queue of the receiver module
    until the receiver is able to handle it with an own task.
    Insertion into the queue happens immediately while the consumption
    depends on the scheduling of the receiver module.
*/

#pragma once

#include <cstddef>
#include <string>
#include "port.h"

// after the constructor of a module has been executed,
// the module status can either be ERROR or STOP
#define MODULE_RUNLEVEL_ERROR -1
#define MODULE_RUNLEVEL_STOP 0
// after running the setup() method the runlevel should change to SETUP_OK
// it could immediately go to OPERATIONAL if no further initializations are necessary
// setup() will be called for all modeules during system build
// if the module does not report a state MODULE_RUNLEVEL_SETUP_OK fter setup()
// it will not be included in the system and reportrd accordingly
#define MODULE_RUNLEVEL_SETUP_OK 1
#define MODULE_RUNLEVEL_INITALIZED 10
// a fully usable module reports OPERATIONAL
// higher numbers can be used to report module-specific information (like link status)
#define MODULE_RUNLEVEL_OPERATIONAL 16
#define MODULE_RUNLEVEL_LINK_OPEN 17

/*
    This is a generic Module class.
    
    All components of the flight stack are represented as modules
    that communicate among each other sending and receiving messages.
    
    Modules sit in the background and are periodically queried by the scheduler
    whether they have some work to do. They will answer based on the
    system clock, on received messages or hardware generated events.
*/
class Module
{

public:

	// This is the constructor of the base class.
	// All derived modules should call it from their constructors.
	Module(std::string name) {
		id = name;
		runlevel_ = MODULE_RUNLEVEL_ERROR;
	};
	
    // All modules have a setup() method that is intended for
    // initializations which cannot easily proceed in the background.
    // This may be reset procedures, configurations or loading
    // calibration data. This will be called once per module
    // before the system loop is started. It will never be called
    // later on, as it could break the system timing.
    // During setup, only messages to system_log are possible.
    virtual void setup() = 0;
    
    // All registered modules receive calls to this interrupt service routine
    // from the 1ms systick interrupt.
    // The interrupt routine should do no significant amount of work.
    // It should take no more than 50us (30000 CPU cycles), otherwise,
    // this will reported as a timing violation to the system log.
    // From the interrupt routine modules can schedule arbitrary
    // worker functions for execution using the schedule_task() call.
    // Every call to a worker function should return in well below a millisecond.
    // If necessary, larger amounts of work need to be distributed over several calls.
    virtual void interrupt() = 0;
    
    // we need a virtual destructor for destroying lists of objects
    virtual ~Module() {};
    
    // query the internal state of the module
    int8_t state() { return runlevel_; };
    
public:
    
    // All modules have a short name which is used to reference the modules
    // when sending messages (telemetry for instance)
    // should have 8 characters at max.
    std::string id;

    // All message ports, that a module may have should be declared public
    // so they can be wired easily during system build
    
protected:

    // All modules have an operational state that encodes for its initialization
    // and setup status and its availability for operations.
    // Zero indicates a broken or out-of-order module, all values above 16
    // indicate a fully functional module.
    // See definitions above
    // Modules may define their own mappings
    int8_t runlevel_;
    
    // All modules have a port over which status messages are sent.
    // This is usually wired to the system log but that's handled during system setup.
    SenderPort status_out;

};
