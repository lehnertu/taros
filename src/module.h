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
    depends on the scheduling of the recever module.
*/

#pragma once

#include <cstddef>

class Module;

/*
    This is a task descriptor
*/
struct Task 
{
    // the module which has started this task
    Module* module;
    // the system time at which the task has been started
    uint32_t schedule_time;
};

struct Message
{
    // the sender of the message
    Module* sender;
    // the receiver of the message
    Module* receiver;
    // an arbitrary data block
    // the data memory is allocated by the sender
    // and released by the receiver after handling
    // TODO: can there be a problem with lost messages ?
    size_t data_size;
    void* data;
};

// TODO:
// we probably need a socket where to send the messages
// describing the queue where the message is stored
// a module can have several queues

class Module
{
public:
private:
};
