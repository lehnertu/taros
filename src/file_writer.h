
#pragma once

#include <string>
#include <SD.h>

#include "module.h"
#include "message.h"
#include "port.h"
#include "stream.h"

/*  
    This is a module for logging messages.
    It writes all received text messages (serialized) to a file.
    
    MODULE_RUNLEVEL_LINK_OPEN indicates that the file has been successfully opened
    and can be written to. If this is not the case all incoming messages are quietly discarded
    and the runlevel is reset to MODULE_RUNLEVEL_OPERATIONAL.
    This information could be used by some supervisor to try and re-open the file by calling setup().
*/
class FileWriter : public Module
{

public:

    // constructor
    FileWriter(
        std::string name,
        std::string file_name);
    
    // here the file is actually opened
    virtual void setup();
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending messages to the bus unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    // it should be called to actually cleanly close the file
    // if this does not happen we try to flush as often as possible,
    // still, data loss may occur
    virtual ~FileWriter();

    // port at which text messages are received to be written to the file
    ReceiverPort in;

private:

    std::string fileName;
    File myFile;
    
    // here are some flags indicating which work is due
    bool  flag_msg_pending;

    // we flush once per second
    uint32_t last_flush;
    
};


/*  
    This is a module for logging streams.
    It writes all received stream to a file.
    It adds a type signature and a timestamp to every dataset.
    
    MODULE_RUNLEVEL_LINK_OPEN indicates that the file has been successfully opened
    and can be written to. If this is not the case all incoming messages are quietly discarded
    and the runlevel is reset to MODULE_RUNLEVEL_OPERATIONAL.
    This information could be used by some supervisor to try and re-open the file by calling setup().
*/
class StreamFileWriter : public Module
{

public:

    // constructor
    StreamFileWriter(
        std::string name,
        std::string file_name);
    
    // here the file is actually opened
    virtual void setup();
    
    // The module is queried by the scheduler every millisecond whether it needs to run.
    // This will return true, when a new dataset has been received.
    virtual bool have_work();

    // This is the worker function being executed by the taskmanager.
    // It writes all pending data blocks to the file unless a limit of execution time is exceeded.
    virtual void run();

    // destructor
    // it should be called to actually cleanly close the file
    // if this does not happen we try to flush as often as possible,
    // still, data loss may occur
    virtual ~StreamFileWriter();

    // ports at which data are received to be written to the file
    StreamReceiver<DATA_IMU_AHRS> ahrs_in;
    StreamReceiver<DATA_IMU_GYRO> gyro_in;

private:

    std::string fileName;
    File myFile;
    
    // here are some flags indicating which work is due
    bool        flag_ahrs_pending;
    bool        flag_gyro_pending;

    // we flush once per second
    uint32_t last_flush;
    
};
