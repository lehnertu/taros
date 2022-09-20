#include "file_writer.h"
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

bool FileWriter::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    if (in.count()>0) flag_msg_pending = true;
    return flag_msg_pending;
}

void FileWriter::run()
{
    while (flag_msg_pending)
    {
        Message msg = in.fetch();
        if (runlevel_== MODULE_RUNLEVEL_LINK_OPEN)
        {
            uint32_t start = micros();
            // write to file
            std::string buffer = msg.printout();
            buffer += std::string("\r\n");
            // write out
            // the write is buffered and returns immediately
            myFile.write(buffer.c_str(), buffer.size());
            uint32_t stop = micros();
            char numStr[20];
            int n = sprintf(numStr,"%d µs\r\n",(int)(stop-start));
            myFile.write(numStr, n);
        };
        flag_msg_pending = (in.count()>0);
    }
    if (FC_elapsed_millis(last_flush) > 1000)
    {
        myFile.flush();
        last_flush = FC_time_now();
    };
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

bool StreamFileWriter::have_work()
{
    // if there is something received in one of the input ports
    // we have to handle it
    flag_ahrs_pending = (ahrs_in.count()>0);
    flag_gyro_pending = (gyro_in.count()>0);
    return flag_ahrs_pending || flag_gyro_pending;
}

void StreamFileWriter::run()
{
    while (flag_ahrs_pending)
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
        // see if there are more messsages
        flag_ahrs_pending = (ahrs_in.count()>0);
    };
    while (flag_gyro_pending)
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
        // see if there are more messsages
        flag_gyro_pending = (gyro_in.count()>0);
    };
    if (FC_elapsed_millis(last_flush) > 1000)
    {
        myFile.flush();
        last_flush = FC_time_now();
    };
}

StreamFileWriter::~StreamFileWriter()
{
    myFile.close();
}

