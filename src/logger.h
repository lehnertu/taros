
#pragma once

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
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // port at which text messages are received to be sent over the USB serial connection
    TimedReceiverPort<MESSAGE_TEXT> text_in;

    // port at which system messages are received to be sent over the USB serial connection
    TimedReceiverPort<MESSAGE_SYSTEM> system_in;

    // port over which formatted messages are sent
    SenderPort<MESSAGE_TEXT> out;

private:

    // The logger has its own serialization routines for timed messages.
    // This serialization skips the sender module name - when sending
    // it will be copied from the incoming message - the logger is just a proxy.
    std::string serialize_message(MESSAGE_TEXT msg, uint32_t time);
    std::string serialize_message(MESSAGE_SYSTEM msg, uint32_t time);

    // here are some flags indicating which work is due
    bool  flag_text_pending;
    bool  flag_system_pending;

};
