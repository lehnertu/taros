#include "file_writer.h"
#include "base.h"
#include "global.h"

FileWriter::FileWriter(
        std::string name,
        std::string file_name)
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_INITALIZED;
    fileName = file_name;
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    last_flush = FC_time_now();
}

void FileWriter::setup()
{
    // open the file
    myFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (myFile)
    {
        runlevel_= MODULE_RUNLEVEL_LINK_OPEN;
        system_log->in.receive(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "file opened.") );
    }
    else
        runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

void FileWriter::interrupt()
{
    if (runlevel_ == MODULE_RUNLEVEL_LINK_OPEN)
    {
        if (in.count()>0)
        {
            Task task = {
                .module = this,
                .schedule_time = FC_time_now(),
                .funct = std::bind(&FileWriter::handle_MSG, this)
                };
            task_list.push_back(task);
        };
        if (FC_elapsed_millis(last_flush) > 1000)
        {
            Task task = {
                .module = this,
                .schedule_time = FC_time_now(),
                .funct = std::bind(&FileWriter::flush, this)
                };
            task_list.push_back(task);
        };
    };
}

void FileWriter::handle_MSG()
{
    if (in.count()>0)
    {
        Message msg = in.fetch();
        // write to file
        std::string buffer = msg.printout();
        buffer += std::string("\r\n");
        // write out
        // the write is buffered and should return immediately
        // if the is data flushed to card it may take longer
        myFile.write(buffer.c_str(), buffer.size());
        // TODO: handle write failures
    };
}

void FileWriter::flush()
{
    myFile.flush();
    last_flush = FC_time_now();
}

FileWriter::~FileWriter()
{
    myFile.close();
}

StreamFileWriter::StreamFileWriter(
        std::string name,
        std::string file_name)
{
    // copy the name
    id = name;
    runlevel_= MODULE_RUNLEVEL_INITALIZED;
    fileName = file_name;
    runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
    last_flush = FC_time_now();
}

void StreamFileWriter::setup()
{
    // open the file
    myFile = SD.open(fileName.c_str(), FILE_WRITE);
    if (myFile)
    {
        runlevel_= MODULE_RUNLEVEL_LINK_OPEN;
        system_log->in.receive(
            Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "file opened.") );
    }
    else
        runlevel_= MODULE_RUNLEVEL_OPERATIONAL;
}

void StreamFileWriter::interrupt()
{
    if (runlevel_ == MODULE_RUNLEVEL_LINK_OPEN)
    {
        if (ahrs_in.count()>0)
        {
            Task task = {
                .module = this,
                .schedule_time = FC_time_now(),
                .funct = std::bind(&StreamFileWriter::handle_AHRS, this)
                };
            task_list.push_back(task);
        };
        if (gyro_in.count()>0)
        {
            Task task = {
                .module = this,
                .schedule_time = FC_time_now(),
                .funct = std::bind(&StreamFileWriter::handle_GYRO, this)
                };
            task_list.push_back(task);
        };
        if (FC_elapsed_millis(last_flush) > 1000)
        {
            Task task = {
                .module = this,
                .schedule_time = FC_time_now(),
                .funct = std::bind(&StreamFileWriter::flush, this)
                };
            task_list.push_back(task);
        };
    };
}

void StreamFileWriter::handle_AHRS()
{
    if (ahrs_in.count()>0)
    {
        // get the message from the queue
        uint8_t sig = DATA_IMU_AHRS_SIGNATURE;
        uint32_t time = FC_time_now();
        DATA_IMU_AHRS data = ahrs_in.fetch();
        if (runlevel_== MODULE_RUNLEVEL_LINK_OPEN)
        {
            myFile.write(&sig,1);
            myFile.write(&time,4);
            myFile.write(&data,12);
        };        
        // TODO: handle write failures
    };
}
    
void StreamFileWriter::handle_GYRO()
{
    if (gyro_in.count()>0)
    {
        // get the message from the queue
        uint8_t sig = DATA_IMU_GYRO_SIGNATURE;
        uint32_t time = FC_time_now();
        DATA_IMU_GYRO data = gyro_in.fetch();
        if (runlevel_== MODULE_RUNLEVEL_LINK_OPEN)
        {
            myFile.write(&sig,1);
            myFile.write(&time,4);
            myFile.write(&data,12);
        };        
        // TODO: handle write failures
    };
}

void StreamFileWriter::flush()
{
    myFile.flush();
    last_flush = FC_time_now();
}

StreamFileWriter::~StreamFileWriter()
{
    myFile.close();
}

