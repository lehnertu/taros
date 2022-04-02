#include <iostream>
#include <fstream>
#include <unistd.h>

#include "global.h"
#include "file_writer.h"

FileWriter::FileWriter(
    std::string name,
    std::string file_name_template)
{
    // copy the name
    id = name;
    // find out the next available sequence number of the file
    int seq = 0;
    int exists = 0;
    do {
        seq++;
        char buffer[10];
        int n = snprintf(buffer, 9, "%03d", seq);
        buffer[n] = '\0';
        file_name = file_name_template + buffer + std::string(".log");
        exists = access( file_name.c_str(), F_OK );
    }
    while (exists == 0);
    flag_message_pending = false;
    // report which file we are writing
    system_log->in.receive(
        Message(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE,
        std::string("writing system_log to ") + file_name) );
}

bool FileWriter::have_work()
{
    if (in.count()>0) flag_message_pending = true;
    return flag_message_pending;
}

void FileWriter::run()
{
    std::ofstream file;
    
    // we have to open and close the file with every write cycle
    // as the program will never stop normally but get interrupted

    // open the file for appending
    file.open (file_name, std::ios::app);
    
    if (file.is_open())
    {        
        while (flag_message_pending)
        {
            Message msg = in.fetch();
            std::string buffer = msg.printout();
            // write out
            file << buffer;
            file << std::endl;
            flag_message_pending = (in.count()>0);
        }
        // close the file
        file.close();
    } else {
        // if we cannot write, discard the messages
        while (flag_message_pending)
        {
            Message msg = in.fetch();
            flag_message_pending = (in.count()>0);
        }
    }
    
}


