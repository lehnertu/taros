#include <iostream>
#include <string>
#include <sstream>

#include "kernel.h"
#include "kernel.h"
#include "commander.h"
#include "util.h"

Commander::Commander(
    std::string name        // the ID of the module
    ) : Module(name)
{
    robot_state = ROBOT_IDLE;
    runlevel_ = MODULE_RUNLEVEL_STOP;
}

void Commander::setup()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_STATE_CHANGE, "initialized.") );
    runlevel_ = MODULE_RUNLEVEL_OPERATIONAL;
};

void Commander::activate()
{
    status_out.transmit(
        Message::SystemMessage(id, FC_time_now(), MSG_LEVEL_MILESTONE, "taking command.") );
    runlevel_ = MODULE_RUNLEVEL_COMMANDER_PIC;
};

void Commander::interrupt()
{
    if (runlevel_ == MODULE_RUNLEVEL_COMMANDER_PIC)
    {
        // If there is something received in one of the input ports we have to handle it.
        // We could do that right here as it takes almost no time
        if (command_in.count()>0)
        {
            schedule_task(this, std::bind(&Commander::handle_uplink, this));
        }
    }
}

void Commander::handle_uplink()
{
    /* TODO: at present we don't do anything
    Message msg = command_in.fetch();
    // process the command messages
    if (msg.type()==MSG_TYPE_COMMAND)
    {
        uint16_t msg_size = msg.size();
        char* msg_body = (char*) msg.get_data();
        // send read-back
        std::stringstream ss;
        ss << "received command : ";
        ss << hexbyte(msg_body[0]);
        ss << hexbyte(msg_body[1]);
        ss << " size = " << msg_size;
        Message read_back = Message::SystemMessage(
            id, FC_time_now(), MSG_LEVEL_READBACK, ss.str());
        status_out.transmit(read_back);        
    };
    */
};

