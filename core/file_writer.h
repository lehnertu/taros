
#pragma once

#include "module.h"
#include "message.h"
#include "port.h"

/* 
    The FileWriter receives text messages and writes them to a file.
*/
class FileWriter : public Module
{

public:

    // constructor
    // the Module gets a name
    // and a template describing the file name to write to
    FileWriter(std::string name, std::string file_name_template);
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset from the GPS has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    virtual ~FileWriter() {};

    // port at which text messages are received to be sent over the USB serial connection
    ReceiverPort<Message_Text> text_in;

private:

    // here are some flags indicating which work is due
    bool  flag_message_pending;
    // the file name we are writing to
    std::string file_name;

};
