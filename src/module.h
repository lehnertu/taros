/*  
    A module is a software object handling one particular task or subsystem
    in the FlightController system. Besides scheduler and taskmanager
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

// after the constructor of a module has been executed,
// the module status can either be ERROR or STOP
#define MODULE_RUNLEVEL_ERROR -1
#define MODULE_RUNLEVEL_STOP 0
// after running the setup() method the runlevel should change to SETUP_OK
// it could immediately go to OPERATIONAL if no further initializations aree necessary
#define MODULE_RUNLEVEL_SETUP_OK 1
#define MODULE_RUNLEVEL_INITALIZED 10
#define MODULE_RUNLEVEL_OPERATIONAL 16
#define MODULE_RUNLEVEL_LINK_OPEN 17

/*
    This is a generic Module class.
    
    All components of the flight stack are represented as modules
    that communicate among each other sending and receiving messages.
    
    Modules sit in the background and are periodically queried by the scheduler
    whether they have some work to do. They will decide based on the
    system clock, on received messages or hardware generated events.
*/
class Module
{

public:

    // All modules have a setup() method that is intended for
    // initializations which cannot easily proceed in the background.
    // This may be reset procedures, configurations or loading
    // calibration data. This will be called once per module
    // before the system loop is started. It will never be called
    // later on, as it could break the system timimg.
    virtual void setup() = 0;
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // If the overall workload is low this can also happen much more often.
    // This method needs to answer and return immediately.
    // No actual work should be performed here,
    // just determined if there is something that needs to be done.
    virtual bool have_work() = 0;
    
    // This is the worker routine that gets executed by the taskmanager.
    // Here all the work of the module should be done.
    // Every call should return in well below a millisecond.
    // If necessary, larger amounts of work need to be distributed into
    // several chunks, that means several calls of run()
    virtual void run() = 0;
    
    // All message ports, that a module may have should be declared public
    // so they can be wired easily during system build
    
    // we need a virtual destructor for destroying lists of objects
    virtual ~Module() {};
    
    int8_t runlevel() { return runlevel_; };
    
public:
    
    // All modules have a short name which is used to reference the modules
    // when sending messages (telemetry for instance)
    // should have 8 characters at max.
    std::string id;

protected:

    // All modules have an operational state that encodes for its initialization
    // and setup status and its availability for operations.
    // Zero indicates a broken or out-of-order module, all values above 16
    // indicate a fully functional module.
    // See definitions above
    // Modules may define their own mappings
    int8_t runlevel_;
    
};
