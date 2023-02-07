
#pragma once

#include <functional>

#include "module.h"
#include "message.h"
#include "port.h"

/* 
    The logger receives a number of possible messages, serializes
    them and sends them as text messages to a number of receivers.
    
    One instance of this class will be created right at system
    start (system_log) that will hold all system messages until
    the taskmanagement is running and these messages can be written
    to a downlink an/or log-file.
*/
class Logger : public Module
{

public:

    // constructor
    Logger(std::string name);
    
    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_OPERATIONAL; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();
    virtual void interrupt() {};
    
    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Logger() {};

    // port at which arbitrary messages are received
    ReceiverPort in;

    // port over which all messages are sent as text messages
    SenderPort text_out;

    // filtered port for system messages only
    SenderPort system_out;
    
private:

    // here are some flags indicating which work is due
    bool  flag_message_pending;

};


/* 
    The Requester queries a number of server messages at a predefined time interval,
    serializes them and sends them as text messages to a number of receivers.
*/
class Requester : public Module
{

public:

    // constructor
    Requester(std::string name, float rate);
    
    // nothing to do
    virtual void setup() { runlevel_ = MODULE_RUNLEVEL_SETUP_OK; };
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~Requester() {};
    
    // Register a callback function of a server, where the logger can request a message.
    // std::function<return_type(list of argument_type(s))>
    // The server method is a function taking no arguments and delivering a message.
    void register_server_callback(std::function<Message(void)> f, std::string name);

    // TODO: The Requester registers all server ports it should log.
    // TODO: The server can provide a number of different message types.

    // port over which formatted messages are sent
    SenderPort out;

private:
    
    // here we store the server callback
    std::string server_name;
    std::function<Message(void)> server_callback;

    // time of the last update
    uint32_t last_update;
    
    // repetition rate of the logging
    float log_rate;
    
    // when we have work to to
    bool flag_update_pending;

};
